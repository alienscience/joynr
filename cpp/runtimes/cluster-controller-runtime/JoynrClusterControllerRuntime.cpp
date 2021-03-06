/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */
#include "JoynrClusterControllerRuntime.h"
#include "joynr/Dispatcher.h"
#include "libjoynr/in-process/InProcessLibJoynrMessagingSkeleton.h"
#include "common/in-process/InProcessMessagingStub.h"
#include "joynr/HttpCommunicationManager.h"
#include "cluster-controller/capabilities-client/ICapabilitiesClient.h"
#include "cluster-controller/http-communication-manager/LongPollMessageSerializer.h"
#include "joynr/CapabilitiesRegistrar.h"
#include "joynr/MessagingSettings.h"
#include "cluster-controller/capabilities-client/FakeCapabilitiesClient.h"
#include "cluster-controller/capabilities-client/CapabilitiesClient.h"
#include "joynr/Request.h"
#include "joynr/exceptions.h"
#include "runtimes/JoynrMetaTypes.h"
#include "libjoynr/in-process/InProcessCapabilitiesStub.h"
#include "cluster-controller/messaging/in-process/InProcessCapabilitiesSkeleton.h"
#include "joynr/CapabilitiesRegistrar.h"
#include "joynr/LocalCapabilitiesDirectory.h"
#include "joynr/InProcessDispatcher.h"
#include "joynr/ConnectorFactory.h"
#include "joynr/SubscriptionManager.h"
#include "joynr/PublicationManager.h"
#include "joynr/InProcessConnectorFactory.h"
#include "joynr/JoynrMessagingConnectorFactory.h"
#include "joynr/ICommunicationManager.h"
#include "joynr/InProcessMessagingAddress.h"
#include "joynr/InProcessPublicationSender.h"
#include "joynr/JoynrMessageSender.h"
#include "joynr/Directory.h"
#include "joynr/infrastructure/GlobalCapabilitiesDirectoryProxy.h"
#include "joynr/LocalChannelUrlDirectory.h"
#include "joynr/SystemServicesSettings.h"
#include "joynr/system/ChannelAddress.h"
#include "libjoynr/in-process/InProcessMessagingStubFactory.h"
#include "libjoynr/joynr-messaging/JoynrMessagingStubFactory.h"

#include <QCoreApplication>
#include <QThread>
#include <QMutex>
#include <cassert>

#ifdef USE_DBUS_COMMONAPI_COMMUNICATION
#include "libjoynr/dbus/DbusMessagingStubFactory.h"
#endif // USE_DBUS_COMMONAPI_COMMUNICATION

namespace joynr {

using namespace joynr_logging;
Logger* JoynrClusterControllerRuntime::logger = Logging::getInstance()->getLogger("JoynrClusterControllerRuntime", "JoynrClusterControllerRuntime");

JoynrClusterControllerRuntime::JoynrClusterControllerRuntime(
        QCoreApplication* app,
        QSettings* settings,
        ICommunicationManager* communicationManager
) :
        JoynrRuntime(*settings),
        joynrDispatcher(NULL),
        inProcessDispatcher(NULL),
        ccDispatcher(NULL),
        publicationManager(NULL),
        subscriptionManager(NULL),
        joynrMessagingSendSkeleton(NULL),
        joynrMessageSender(NULL),
        app(app),
        capabilitiesClient(NULL),
        messagingEndpointDirectory(new Directory<QString, joynr::system::Address>(QString("JoynrClusterControllerRuntime-MessagingEndpointDirectory"))),
        localCapabilitiesDirectory(NULL),
        channelUrlDirectory(),
        capabilitiesSkeleton(NULL),
        cache(),
        channelUrlDirectoryProxy(NULL),
        libJoynrMessagingSkeleton(NULL),
        communicationManager(communicationManager),
        longpollMessageSerializer(NULL),
        dispatcherList(),
        inProcessConnectorFactory(NULL),
        inProcessPublicationSender(NULL),
        joynrMessagingConnectorFactory(NULL),
        connectorFactory(NULL),
        settings(settings),
        messagingSettings(NULL),
        libjoynrSettings(NULL)
#ifdef USE_DBUS_COMMONAPI_COMMUNICATION
        , dbusSettings(NULL)
        , ccDbusMessageRouterAdapter(NULL)
        , ccDbusCapabilitiesAdapter(NULL)
#endif // USE_DBUS_COMMONAPI_COMMUNICATION

{
    /*
      * WARNING - metatypes are not registered yet here.
      */

    //This is a workaround to register the Metatypes for providerQos.
    //Normally a new datatype is registered in all datatypes that use the new datatype.
    //However, when receiving a datatype as a returnValue of a RPC, the constructor has never been called before
    //so the datatype is not registered, and cannot be deserialized.


    registerJoynrMetaTypes();
    initializeAllDependencies();
}

void JoynrClusterControllerRuntime::initializeAllDependencies(){
    /**
      * libjoynr side skeleton & dispatcher
      * This needs some preparation of libjoynr and clustercontroller parts.
      */
    messagingSettings = new MessagingSettings(*settings);
    messagingSettings->printSettings();
    libjoynrSettings = new LibjoynrSettings(*settings);
    libjoynrSettings->printSettings();

    //CAREFUL: the factory creates an old style dispatcher, not the new one!

    inProcessDispatcher = new InProcessDispatcher();
    /* CC */
    // create the messaging stub factory
    MessagingStubFactory* messagingStubFactory = new MessagingStubFactory();
    #ifdef USE_DBUS_COMMONAPI_COMMUNICATION
    messagingStubFactory->registerStubFactory(new DbusMessagingStubFactory());
    #endif // USE_DBUS_COMMONAPI_COMMUNICATION
    messagingStubFactory->registerStubFactory(new InProcessMessagingStubFactory());
    // init message router
    messageRouter = QSharedPointer<MessageRouter>(new MessageRouter(messagingEndpointDirectory, messagingStubFactory));
    // provision global capabilities directory
    QSharedPointer<joynr::system::Address> endpointAddressCapa(
                new system::ChannelAddress(messagingSettings->getCapabilitiesDirectoryChannelId())
    );
    messageRouter->addProvisionedNextHop(messagingSettings->getCapabilitiesDirectoryParticipantId(), endpointAddressCapa);
    // provision channel url directory
    QSharedPointer<joynr::system::Address> endpointAddressChannel(
                new system::ChannelAddress(messagingSettings->getChannelUrlDirectoryChannelId())
    );
    messageRouter->addProvisionedNextHop(messagingSettings->getChannelUrlDirectoryParticipantId(), endpointAddressChannel);

    /* LibJoynr */
    assert(messageRouter);
    joynrMessageSender = new JoynrMessageSender(messageRouter);
    joynrDispatcher = new Dispatcher(joynrMessageSender);
    joynrMessageSender->registerDispatcher(joynrDispatcher);

    /* CC */
    //TODO: libjoynrmessagingskeleton now uses the Dispatcher, should it use the InprocessDispatcher?
    libJoynrMessagingSkeleton = QSharedPointer<InProcessMessagingSkeleton> (new InProcessLibJoynrMessagingSkeleton(joynrDispatcher));
    //EndpointAddress to messagingStub is transmitted when a provider is registered
    //messagingStubFactory->registerInProcessMessagingSkeleton(libJoynrMessagingSkeleton);

    /**
      * ClusterController side
      *
      */
    if(communicationManager.isNull()) {
        LOG_INFO(logger, "The communication manager supplied is NULL, creating the default HttpCommunicationManager");
        communicationManager = QSharedPointer<ICommunicationManager>(new HttpCommunicationManager(*messagingSettings));
    }

    QString channelId = communicationManager->getReceiveChannelId();
    longpollMessageSerializer = new LongPollMessageSerializer(messageRouter, messagingEndpointDirectory);
    communicationManager->setMessageDispatcher(longpollMessageSerializer); // LongpollingMessageReceiver will call the messageRouter when data received
    messagingStubFactory->registerStubFactory(new JoynrMessagingStubFactory(communicationManager));

    //joynrMessagingSendSkeleton = new DummyClusterControllerMessagingSkeleton(messageRouter);
    //ccDispatcher = DispatcherFactory::createDispatcherInSameThread(messagingSettings);

    //we currently have to use the fake client, because JAVA side is not yet working for CapabilitiesServer.
    bool usingRealCapabilitiesClient = /*when switching this to true, turn on the UUID in systemintegrationtests again*/ true;
    capabilitiesClient = new CapabilitiesClient(channelId);// ownership of this is not transferred
    //try using the real capabilitiesClient again:
    //capabilitiesClient = new CapabilitiesClient(channelId);// ownership of this is not transferred

    localCapabilitiesDirectory = new LocalCapabilitiesDirectory(*messagingSettings, capabilitiesClient, messagingEndpointDirectory);
    capabilitiesSkeleton = new InProcessCapabilitiesSkeleton(messagingEndpointDirectory, localCapabilitiesDirectory, channelId);

#ifdef USE_DBUS_COMMONAPI_COMMUNICATION
    dbusSettings = new DbusSettings(*settings);
    dbusSettings->printSettings();
    // register dbus skeletons for capabilities and messaging interfaces
    QString ccMessagingAddress(dbusSettings->createClusterControllerMessagingAddressString());
    ccDbusMessageRouterAdapter = new DBusMessageRouterAdapter(*messageRouter, ccMessagingAddress);
    QString ccCapabilitiesAddress(dbusSettings->createClusterControllerCapabilitiesAddressString());
    ccDbusCapabilitiesAdapter = new DbusCapabilitiesAdapter(*messagingEndpointDirectory, *localCapabilitiesDirectory, ccCapabilitiesAddress, communicationManager->getReceiveChannelId());
#endif // USE_DBUS_COMMONAPI_COMMUNICATION

    /**
      * libJoynr side
      *
      */
    publicationManager = new PublicationManager();
    subscriptionManager = new SubscriptionManager();
    inProcessPublicationSender = new InProcessPublicationSender(subscriptionManager);
    QSharedPointer<joynr::system::Address> messagingEndpointAddress(new InProcessMessagingAddress(libJoynrMessagingSkeleton));
    //subscriptionManager = new SubscriptionManager(...)
    inProcessConnectorFactory = new InProcessConnectorFactory(subscriptionManager, publicationManager, inProcessPublicationSender);
    joynrMessagingConnectorFactory = new JoynrMessagingConnectorFactory(joynrMessageSender, subscriptionManager);

    connectorFactory = createConnectorFactory(inProcessConnectorFactory, joynrMessagingConnectorFactory);

    joynrCapabilitiesSendStub = new InProcessCapabilitiesStub(capabilitiesSkeleton);
    proxyFactory = new ProxyFactory(joynrCapabilitiesSendStub, messagingEndpointAddress, connectorFactory, &cache);

    capabilitiesAggregator = QSharedPointer<CapabilitiesAggregator>(new CapabilitiesAggregator(joynrCapabilitiesSendStub, dynamic_cast<IRequestCallerDirectory*>(inProcessDispatcher)));
    dispatcherList.append(joynrDispatcher);
    dispatcherList.append(inProcessDispatcher);

    // Set up the persistence file for storing provider participant ids
    QString persistenceFilename = libjoynrSettings->getParticipantIdsPersistenceFilename();
    participantIdStorage = QSharedPointer<ParticipantIdStorage>(new ParticipantIdStorage(persistenceFilename));

    dispatcherAddress = messagingEndpointAddress;
    capabilitiesRegistrar =  new CapabilitiesRegistrar(dispatcherList,
                                                       qSharedPointerDynamicCast<ICapabilities>(capabilitiesAggregator),
                                                       messagingEndpointAddress,
                                                       participantIdStorage,
                                                       dispatcherAddress,
                                                       messageRouter);

    joynrDispatcher->registerPublicationManager(publicationManager);
    joynrDispatcher->registerSubscriptionManager(subscriptionManager);

    /**
     * Finish initialising Capabilitiesclient by building a Proxy and passing it
     */


    if (usingRealCapabilitiesClient)
    {
        ProxyBuilder<infrastructure::GlobalCapabilitiesDirectoryProxy>* capabilitiesProxyBuilder =
                getProxyBuilder<infrastructure::GlobalCapabilitiesDirectoryProxy>(
                    messagingSettings->getDiscoveryDirectoriesDomain()
                );
        DiscoveryQos discoveryQos(10000);
        discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY); //actually only one provider should be available
        QSharedPointer<infrastructure::GlobalCapabilitiesDirectoryProxy> cabilitiesProxy (
            capabilitiesProxyBuilder
                ->setRuntimeQos(MessagingQos(40000)) //TODO magic values.
                ->setCached(true)
                ->setDiscoveryQos(discoveryQos)
                ->build()
            );
        ((CapabilitiesClient*)capabilitiesClient)->init(cabilitiesProxy);
    }

    ProxyBuilder<infrastructure::ChannelUrlDirectoryProxy>* channelUrlDirectoryProxyBuilder =
            getProxyBuilder<infrastructure::ChannelUrlDirectoryProxy>(
                messagingSettings->getDiscoveryDirectoriesDomain()
            );

    DiscoveryQos discoveryQos(10000);
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY); //actually only one provider should be available
    channelUrlDirectoryProxy = QSharedPointer<infrastructure::ChannelUrlDirectoryProxy> (
          channelUrlDirectoryProxyBuilder
                ->setRuntimeQos(MessagingQos(15000)) //TODO magic values.
                ->setCached(true)
                ->setDiscoveryQos(discoveryQos)
                ->build()
           );

    channelUrlDirectory = QSharedPointer<ILocalChannelUrlDirectory>(
        new LocalChannelUrlDirectory(*messagingSettings, channelUrlDirectoryProxy)
        );
    communicationManager.dynamicCast<HttpCommunicationManager>()->init(channelUrlDirectory);

}

ConnectorFactory* JoynrClusterControllerRuntime::createConnectorFactory(
        InProcessConnectorFactory* inProcessConnectorFactory,
        JoynrMessagingConnectorFactory* joynrMessagingConnectorFactory)
{
    return new ConnectorFactory(inProcessConnectorFactory, joynrMessagingConnectorFactory);
}

void JoynrClusterControllerRuntime::registerRoutingProvider()
{
    QString domain(systemServicesSettings.getDomain());
    QSharedPointer<joynr::system::RoutingProvider> routingProvider(messageRouter);
    QString interfaceName(routingProvider->getInterfaceName());
    QString authToken(systemServicesSettings.getCcRoutingProviderAuthenticationToken());
    QString participantId(systemServicesSettings.getCcRoutingProviderParticipantId());

    // provision the participant ID for the routing provider
    participantIdStorage->setProviderParticipantId(domain, interfaceName, authToken, participantId);

    registerCapability<joynr::system::RoutingProvider>(domain, routingProvider, authToken);
}

JoynrClusterControllerRuntime::~JoynrClusterControllerRuntime() {
    LOG_TRACE(logger, "entering ~JoynrClusterControllerRuntime");
    //joynrDispatcher needs to be deleted before messagingEndpointdirectory, because the dispatcherthreadpool
    //might access the messageRouter, which might acces the messagingEndpointdirectory.
    if(joynrDispatcher != NULL){
        LOG_TRACE(logger, "joynrDispatcher");
        //joynrDispatcher->stopMessaging();
        delete joynrDispatcher;
    }

    delete inProcessDispatcher;
    inProcessDispatcher = NULL;
    delete localCapabilitiesDirectory;
    localCapabilitiesDirectory = NULL;
    delete capabilitiesClient;
    capabilitiesClient = NULL;

    delete messagingEndpointDirectory;
    messagingEndpointDirectory = NULL;
//    //~HttpCommunicationmanager will delete longpollmessageserializer, but MockCommunicationMgr will not delete it.
//    //thus check if it has been set to NULL, and if not, try to delete it again.
//    if (longpollMessageSerializer) {
//        delete longpollMessageSerializer;
//    }
    delete inProcessPublicationSender;
    inProcessPublicationSender = NULL;
    delete joynrMessageSender;
    delete proxyFactory;
    delete messagingSettings;
    delete libjoynrSettings;
    delete capabilitiesRegistrar;

#ifdef USE_DBUS_COMMONAPI_COMMUNICATION
    delete ccDbusMessageRouterAdapter;
    delete ccDbusCapabilitiesAdapter;
    delete dbusSettings;
#endif // USE_DBUS_COMMONAPI_COMMUNICATION
    settings->clear();
    settings->deleteLater();

    LOG_TRACE(logger, "leaving ~JoynrClusterControllerRuntime");
}

void JoynrClusterControllerRuntime::startMessaging() {
//    assert(joynrDispatcher!=NULL);
//    joynrDispatcher->startMessaging();
//    joynrDispatcher->waitForMessaging();
    assert(communicationManager!=NULL);
    communicationManager->startReceiveQueue();
}

void JoynrClusterControllerRuntime::stopMessaging() {
    //joynrDispatcher->stopMessaging();
    communicationManager->stopReceiveQueue();
}

void JoynrClusterControllerRuntime::runForever() {
    app->exec();
}

JoynrClusterControllerRuntime *JoynrClusterControllerRuntime::create(QSettings* settings)
{
    int argc = 0;
    char *argv[] = {0};
    QCoreApplication* coreApplication = new QCoreApplication(argc, argv);
    JoynrClusterControllerRuntime* runtime = new JoynrClusterControllerRuntime(coreApplication, settings);
    runtime->startMessaging();
    runtime->waitForChannelCreation();
    runtime->registerRoutingProvider();
    return runtime;
}

void JoynrClusterControllerRuntime::unregisterCapability(QString participantId) {
    assert(capabilitiesRegistrar);
    capabilitiesRegistrar->unregisterCapability(participantId);
}

void JoynrClusterControllerRuntime::waitForChannelCreation() {
    communicationManager->waitForReceiveQueueStarted();
}

void JoynrClusterControllerRuntime::deleteChannel() {
   communicationManager->tryToDeleteChannel();
}

} // namespace joynr
