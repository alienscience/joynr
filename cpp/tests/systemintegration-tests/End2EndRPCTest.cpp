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
#include "joynr/MessagingSettings.h"
#include "joynr/SettingsMerger.h"
#include "joynr/HttpCommunicationManager.h"
#include "tests/utils/MockObjects.h"
#include "joynr/tests/TestProvider.h"
#include "joynr/tests/TestProxy.h"
#include "joynr/vehicle/GpsProxy.h"
#include "joynr/types/ProviderQos.h"
#include "joynr/RequestStatus.h"
#include "joynr/Future.h"
#include "joynr/OnChangeWithKeepAliveSubscriptionQos.h"

using namespace ::testing;


using namespace joynr;

class End2EndRPCTest : public Test{
public:
    QString domain;
    JoynrClusterControllerRuntime* runtime;
    QSharedPointer<vehicle::GpsProvider> gpsProvider;

    End2EndRPCTest() :
        domain(),
        runtime(NULL)
    {
        QSettings* settings = SettingsMerger::mergeSettings(QString("test-resources/integrationtest.settings"));
        SettingsMerger::mergeSettings(QString("test-resources/libjoynrintegrationtest.settings"), settings);
        runtime = new JoynrClusterControllerRuntime(NULL, settings);
        //This is a workaround to register the Metatypes for providerQos.
        //Normally a new datatype is registered in all datatypes that use the new datatype.
        //However, when receiving a datatype as a returnValue of a RPC, the constructor has never been called before
        //so the datatype is not registered, and cannot be deserialized.
        qRegisterMetaType<joynr::types::ProviderQos>("joynr::types::ProviderQos");
        qRegisterMetaType<joynr__types__ProviderQos>("joynr__types__ProviderQos");

        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.mid(1,uuid.length()-2);
        domain = "cppEnd2EndRPCTest_Domain_" + uuid;
    }
    // Sets up the test fixture.
    void SetUp(){
       runtime->startMessaging();
       runtime->waitForChannelCreation();
    }

    // Tears down the test fixture.
    void TearDown(){
        runtime->deleteChannel(); //cleanup the channels so they dont remain on the bp
        runtime->stopMessaging();

        // Remove participant id persistence file
        QFile::remove(LibjoynrSettings::DEFAULT_PARTICIPANT_IDS_PERSISTENCE_FILENAME());

        QThreadSleep::msleep(550);
    }

    ~End2EndRPCTest(){
        delete runtime;
    }
private:
    DISALLOW_COPY_AND_ASSIGN(End2EndRPCTest);
};

// leadsm to assert failure in GpsInProcessConnector line 185: not yet implemented in connector
TEST_F(End2EndRPCTest, call_rpc_method_and_get_expected_result)
{

    QSharedPointer<MockGpsProvider> mockProvider(new MockGpsProvider());
    types::GpsLocation gpsLocation1(1.1, 2.2, 3.3, types::GpsFixEnum::MODE2D, 0.0, 0.0, 0.0, 0.0, 444, 444, 4);

    runtime->registerCapability<vehicle::GpsProvider>(domain, mockProvider, QString());
    QThreadSleep::msleep(550);

    ProxyBuilder<vehicle::GpsProxy>* gpsProxyBuilder = runtime->getProxyBuilder<vehicle::GpsProxy>(domain);
    DiscoveryQos discoveryQos;
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);
    discoveryQos.setDiscoveryTimeout(1000);

    qlonglong qosRoundTripTTL = 40000;
    qlonglong qosCacheDataFreshnessMs = 400000;
    QSharedPointer<vehicle::GpsProxy> gpsProxy(gpsProxyBuilder
            ->setRuntimeQos(MessagingQos(qosRoundTripTTL))
            ->setProxyQos(ProxyQos(qosCacheDataFreshnessMs))
            ->setCached(false)
            ->setDiscoveryQos(discoveryQos)
            ->build());
    QSharedPointer<Future<int> >gpsFuture (new Future<int>());
    gpsProxy->calculateAvailableSatellites(gpsFuture);
    gpsFuture->waitForFinished();
    int expectedValue = 42; //as defined in MockGpsProvider
    EXPECT_EQ(expectedValue, gpsFuture->getValue());
    //TODO CA: shared pointer for proxy builder?
    delete gpsProxyBuilder;
    // This is not yet implemented in CapabilitiesClient
    // runtime->unregisterCapability("Fake_ParticipantId_vehicle/gpsDummyProvider");
}

TEST_F(End2EndRPCTest, call_void_operation)
{

    QSharedPointer<MockTestProvider> mockProvider(new MockTestProvider(types::ProviderQos(QList<types::CustomParameter>(),1,1,types::ProviderScope::GLOBAL,false)));

    runtime->registerCapability<tests::TestProvider>(domain, mockProvider, QString());
    QThreadSleep::msleep(550);

    ProxyBuilder<tests::TestProxy>* testProxyBuilder = runtime->getProxyBuilder<tests::TestProxy>(domain);
    DiscoveryQos discoveryQos;
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);
    discoveryQos.setDiscoveryTimeout(1000);

    qlonglong qosRoundTripTTL = 40000;
    qlonglong qosCacheDataFreshnessMs = 400000;
    tests::TestProxy* testProxy = testProxyBuilder
            ->setRuntimeQos(MessagingQos(qosRoundTripTTL))
            ->setProxyQos(ProxyQos(qosCacheDataFreshnessMs))
            ->setCached(false)
            ->setDiscoveryQos(discoveryQos)
            ->build();
    RequestStatus status;
    testProxy->voidOperation(status);
//    EXPECT_EQ(expectedValue, gpsFuture->getValue());
    //TODO CA: shared pointer for proxy builder?
    delete testProxy;
    delete testProxyBuilder;
    // This is not yet implemented in CapabilitiesClient
    // runtime->unregisterCapability("Fake_ParticipantId_vehicle/gpsDummyProvider");
}

// tests in process subscription
TEST_F(End2EndRPCTest, _call_subscribeTo_and_get_expected_result)
{
    QSharedPointer<MockTestProvider> mockProvider(new MockTestProvider());
    types::GpsLocation gpsLocation1(1.1, 2.2, 3.3, types::GpsFixEnum::MODE2D, 0.0, 0.0, 0.0, 0.0, 444, 444, 4);
    runtime->registerCapability<tests::TestProvider>(domain, mockProvider, QString());

    QThreadSleep::msleep(550);

    ProxyBuilder<tests::TestProxy>* testProxyBuilder =
            runtime->getProxyBuilder<tests::TestProxy>(domain);
    DiscoveryQos discoveryQos;
    discoveryQos.setArbitrationStrategy(DiscoveryQos::ArbitrationStrategy::HIGHEST_PRIORITY);
    discoveryQos.setDiscoveryTimeout(1000);

    qlonglong qosRoundTripTTL = 40000;
    qlonglong qosCacheDataFreshnessMs = 400000;
    QSharedPointer<tests::TestProxy> testProxy(testProxyBuilder
            ->setRuntimeQos(MessagingQos(qosRoundTripTTL))
            ->setProxyQos(ProxyQos(qosCacheDataFreshnessMs))
            ->setCached(false)
            ->setDiscoveryQos(discoveryQos)
            ->build());

    MockGpsSubscriptionListener* mockListener = new MockGpsSubscriptionListener();
    QSharedPointer<ISubscriptionListener<types::GpsLocation> > subscriptionListener(
                mockListener);

    EXPECT_CALL(*mockListener, receive(A<types::GpsLocation>()))
            .Times(AtLeast(2));

    auto subscriptionQos = QSharedPointer<SubscriptionQos>(new OnChangeWithKeepAliveSubscriptionQos(
                800, // validity_ms
                100, // minInterval_ms
                200, // maxInterval_ms
                1000 // alertInterval_ms
    ));
    testProxy->subscribeToLocation(subscriptionListener, subscriptionQos);
    QThreadSleep::msleep(1500);
    //TODO CA: shared pointer for proxy builder?
    delete testProxyBuilder;
    // This is not yet implemented in CapabilitiesClient
    // runtime->unregisterCapability("Fake_ParticipantId_vehicle/gpsDummyProvider");
}
