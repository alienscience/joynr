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
#include "joynr/PrivateCopyAssign.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/utils/MockObjects.h"
#include "runtimes/cluster-controller-runtime/JoynrClusterControllerRuntime.h"
#include "joynr/HttpCommunicationManager.h"
#include "joynr/vehicle/GpsProxy.h"
#include "joynr/tests/TestProxy.h"
#include "joynr/types/Trip.h"
#include "joynr/types/GpsLocation.h"
#include "joynr/CapabilitiesRegistrar.h"
#include "utils/QThreadSleep.h"
#include "PrettyPrint.h"
#include "cluster-controller/capabilities-client/CapabilitiesClient.h"
#include "joynr/infrastructure/IGlobalCapabilitiesDirectory.h"
#include "joynr/LocalCapabilitiesDirectory.h"
#include "cluster-controller/capabilities-client/IGlobalCapabilitiesCallback.h"
#include "joynr/joynrlogging.h"
#include "cluster-controller/messaging/MessagingPropertiesPersistence.h"
#include "joynr/SettingsMerger.h"

using namespace ::testing;
using namespace joynr;

ACTION_P(ReleaseSemaphore,semaphore)
{
    semaphore->release(1);
}

static const QString messagingPropertiesPersistenceFileName("CapabilitiesClientTest-joynr.settings");
static const QString settingsFilename("test-resources/SystemIntegrationTest1.settings");
static const QString libJoynrSettingsFilename("test-resources/libjoynrSystemIntegration1.settings");

class CapabilitiesClientTest : public Test {
public:
    joynr_logging::Logger* logger;
    JoynrClusterControllerRuntime* runtime;
    QSettings settings;
    MessagingSettings messagingSettings;
    QString channelId;

    CapabilitiesClientTest() :
        logger(joynr_logging::Logging::getInstance()->getLogger("TEST", "CapabilitiesClientTest")),
        runtime(NULL),
        settings(settingsFilename, QSettings::IniFormat),
        messagingSettings(settings)
    {
        messagingSettings.setMessagingPropertiesPersistenceFilename(messagingPropertiesPersistenceFileName);
        MessagingPropertiesPersistence storage(messagingSettings.getMessagingPropertiesPersistenceFilename());
        channelId = storage.getChannelId();
        QSettings* settings = SettingsMerger::mergeSettings(settingsFilename);
        SettingsMerger::mergeSettings(libJoynrSettingsFilename, settings);
        runtime = new JoynrClusterControllerRuntime(NULL, settings,
                                           new HttpCommunicationManager(messagingSettings));
    }

    void SetUp() {
        runtime->startMessaging();
        runtime->waitForChannelCreation();
    }

    void TearDown() {
        runtime->deleteChannel(); //cleanup the channels so they dont remain on the bp
        runtime->stopMessaging();
    }

    ~CapabilitiesClientTest(){
        delete runtime;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(CapabilitiesClientTest);

};

TEST_F(CapabilitiesClientTest, registerAndRetrieveCapability) {
    LOG_TRACE(logger, "Waiting for Channel creation");
    runtime->waitForChannelCreation();
    LOG_TRACE(logger, "Finished Waiting for Channel creation");
    CapabilitiesClient* capabilitiesClient = new CapabilitiesClient(channelId);// ownership of this is not transferred
    ProxyBuilder<infrastructure::GlobalCapabilitiesDirectoryProxy>* capabilitiesProxyBuilder =
            runtime->getProxyBuilder<infrastructure::GlobalCapabilitiesDirectoryProxy>(
                messagingSettings.getDiscoveryDirectoriesDomain()
            );
    DiscoveryQos discoveryQos;
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY); //actually only one provider should be available
    QSharedPointer<infrastructure::GlobalCapabilitiesDirectoryProxy> cabilitiesProxy (
        capabilitiesProxyBuilder
            ->setRuntimeQos(MessagingQos(10000)) //TODO magic values.
            ->setCached(true)
            ->setDiscoveryQos(discoveryQos)
            ->build()
        );
    capabilitiesClient->init(cabilitiesProxy);

    QList<types::CapabilityInformation> capabilitiesInformationList;
    QString capDomain("testDomain");
    QString capInterface("testInterface");
    types::ProviderQos capProviderQos;
    QString capChannelId("testChannelId");
    QString capParticipantId("testParticipantId");

    capabilitiesInformationList.append(types::CapabilityInformation(capDomain, capInterface, capProviderQos, capChannelId, capParticipantId));
    LOG_DEBUG(logger,"Registering capabilities");
    capabilitiesClient->registerCapabilities(capabilitiesInformationList);
    LOG_DEBUG(logger,"Registered capabilities");
    //sync methods are not yet implemented
//    QList<types::CapabilityInformation> capResultList = capabilitiesClient->getCapabilitiesForInterfaceAddress(capDomain, capInterface);
//    EXPECT_EQ(capResultList, capabilitiesInformationList);
    QSharedPointer<IGlobalCapabilitiesCallbackMock> callback(new IGlobalCapabilitiesCallbackMock());

    // use a semaphore to wait for capabilities to be received
    QSemaphore semaphore(0);
    EXPECT_CALL(*callback, capabilitiesReceived(A<QList<types::CapabilityInformation> >()))
            .WillRepeatedly(ReleaseSemaphore(&semaphore));

    LOG_DEBUG(logger,"get capabilities");
    capabilitiesClient->getCapabilitiesForInterfaceAddress(capDomain, capInterface, callback);
    semaphore.tryAcquire(1,10000);
    LOG_DEBUG(logger,"finished get capabilities");

    delete capabilitiesProxyBuilder;
}

