package io.joynr.integration;

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

import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import io.joynr.arbitration.ArbitrationStrategy;
import io.joynr.arbitration.DiscoveryQos;
import io.joynr.exceptions.JoynrArbitrationException;
import io.joynr.exceptions.JoynrIllegalStateException;
import io.joynr.integration.util.DummyJoynrApplication;
import io.joynr.integration.util.ServersUtil;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.MessagingQos;
import io.joynr.proxy.ProxyBuilder;
import io.joynr.pubsub.PubSubTestProviderImpl;
import io.joynr.pubsub.SubscriptionQos;
import io.joynr.pubsub.subscription.SubscriptionListener;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.JoynrInjectorFactory;
import io.joynr.runtime.PropertyLoader;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;
import java.util.UUID;

import joynr.OnChangeSubscriptionQos;
import joynr.OnChangeWithKeepAliveSubscriptionQos;
import joynr.PeriodicSubscriptionQos;
import joynr.tests.TestProxy;
import joynr.types.GpsLocation;

import org.eclipse.jetty.server.Server;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.mockito.runners.MockitoJUnitRunner;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@RunWith(MockitoJUnitRunner.class)
public class SubscriptionEnd2EndTest {
    private static final Logger logger = LoggerFactory.getLogger(SubscriptionEnd2EndTest.class);

    private static final long expected_latency_ms = 50;

    @Rule
    public TestName name = new TestName();

    private static PubSubTestProviderImpl provider;
    private static String domain;
    private static TestProxy proxy;

    // private SubscriptionQos subscriptionQos;
    private static DummyJoynrApplication providingApplication;
    private static DummyJoynrApplication consumingApplication;

    private int period_ms = 200;

    private static Server server;

    @BeforeClass
    public static void setupEndpoints() throws Exception {
        server = ServersUtil.startServers();
        domain = "TestDomain" + System.currentTimeMillis();
        setupProvidingApplication();
        setupConsumingApplication();
    }

    @Before
    public void setUp() throws JoynrArbitrationException, InterruptedException, IOException {
        Thread.sleep(200);
        Object methodName = name.getMethodName();
        logger.info("Starting {} ...", methodName);
    }

    @AfterClass
    public static void tearDownEndpoints() throws Exception {
        providingApplication.shutdown();
        providingApplication = null;
        Thread.sleep(200);
        consumingApplication.shutdown();
        consumingApplication = null;
        server.stop();
    }

    private static void setupProvidingApplication() {
        Properties factoryPropertiesProvider;

        String channelIdProvider = "JavaTest-" + UUID.randomUUID().getLeastSignificantBits()
                + "-Provider-SubscriptionEnd2EndTest";

        factoryPropertiesProvider = PropertyLoader.loadProperties("testMessaging.properties");
        factoryPropertiesProvider.put(MessagingPropertyKeys.CHANNELID, channelIdProvider);
        factoryPropertiesProvider.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());
        factoryPropertiesProvider.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain-"
                + UUID.randomUUID().toString());
        providingApplication = (DummyJoynrApplication) new JoynrInjectorFactory(factoryPropertiesProvider).createApplication(DummyJoynrApplication.class);

        provider = new PubSubTestProviderImpl();
        providingApplication.getRuntime().registerCapability(domain,
                                                             provider,
                                                             joynr.tests.TestSync.class,
                                                             "SubscriptionEnd2End");
    }

    private static void setupConsumingApplication() throws JoynrArbitrationException, JoynrIllegalStateException,
                                                   InterruptedException {
        String channelIdConsumer = "JavaTest-" + UUID.randomUUID().getLeastSignificantBits()
                + "-Consumer-SubscriptionEnd2EndTest";

        Properties factoryPropertiesB = PropertyLoader.loadProperties("testMessaging.properties");
        factoryPropertiesB.put(MessagingPropertyKeys.CHANNELID, channelIdConsumer);
        factoryPropertiesB.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());
        factoryPropertiesB.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain-"
                + UUID.randomUUID().toString());

        consumingApplication = (DummyJoynrApplication) new JoynrInjectorFactory(factoryPropertiesB).createApplication(DummyJoynrApplication.class);

        MessagingQos messagingQos = new MessagingQos(5000);
        DiscoveryQos discoveryQos = new DiscoveryQos(5000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);

        ProxyBuilder<TestProxy> proxyBuilder = consumingApplication.getRuntime()
                                                                   .getProxyBuilder(domain, joynr.tests.TestProxy.class);
        proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        // Wait until all the registrations and lookups are finished, to make
        // sure the timings are correct once the tests start by sending a sync
        // request to the test-proxy
        proxy.getFirstPrime();
        logger.trace("Sync call to proxy finished");

    }

    @Test
    public void testSystemPropertiesUnchanged() {
        assertTrue(System.getProperty(ConfigurableMessagingSettings.PROPERTY_SEND_MSG_RETRY_INTERVAL_MS) == null);
        assertTrue(System.getProperty(ConfigurableMessagingSettings.PROPERTY_DISCOVERY_REQUEST_TIMEOUT) == null);
    }

    @Test
    @SuppressWarnings("unchecked")
    public void registerSubscriptionAndReceiveUpdates() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        int subscriptionDuration = (period_ms * 4);
        long alertInterval_ms = 500;
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;
        SubscriptionQos subscriptionQos = new PeriodicSubscriptionQos(period_ms, expiryDate_ms, alertInterval_ms, 0);
        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQos);
        Thread.sleep(subscriptionDuration);
        verify(integerListener, times(0)).publicationMissed();
        // TODO verify publications shipped correct data
        verify(integerListener, times(1)).receive(eq(42));
        verify(integerListener, times(1)).receive(eq(43));
        verify(integerListener, times(1)).receive(eq(44));
        verify(integerListener, times(1)).receive(eq(45));

        proxy.unsubscribeFromTestAttribute(subscriptionId);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void registerSubscriptionForComplexDatatype() throws InterruptedException {
        SubscriptionListener<GpsLocation> gpsListener = mock(SubscriptionListener.class);
        int subscriptionDuration = (period_ms * 4);
        long alertInterval_ms = 500;
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;
        SubscriptionQos subscriptionQos = new PeriodicSubscriptionQos(period_ms, expiryDate_ms, alertInterval_ms, 0);

        String subscriptionId = proxy.subscribeToComplexTestAttribute(gpsListener, subscriptionQos);
        Thread.sleep(subscriptionDuration);
        // 100 2100 4100 6100
        verify(gpsListener, times(0)).publicationMissed();
        verify(gpsListener, atLeast(4)).receive(eq(provider.getComplexTestAttribute()));

        proxy.unsubscribeFromComplexTestAttribute(subscriptionId);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void registerSubscriptionForListAndReceiveUpdates() throws InterruptedException {
        SubscriptionListener<List<Integer>> integerListListener = mock(SubscriptionListener.class);
        provider.setTestAttribute(42);

        int subscriptionDuration = (period_ms * 3);
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;
        SubscriptionQos subscriptionQos = new PeriodicSubscriptionQos(period_ms, expiryDate_ms, 0, 0);

        String subscriptionId = proxy.subscribeToListOfInts(integerListListener, subscriptionQos);
        Thread.sleep(subscriptionDuration);
        verify(integerListListener, times(0)).publicationMissed();

        verify(integerListListener, times(1)).receive(eq(Arrays.asList(42)));
        verify(integerListListener, times(1)).receive(eq(Arrays.asList(42, 43)));
        verify(integerListListener, times(1)).receive(eq(Arrays.asList(42, 43, 44)));
        verifyNoMoreInteractions(integerListListener);

        proxy.unsubscribeFromListOfInts(subscriptionId);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void registerAndStopSubscription() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);
        int subscriptionDuration = (period_ms * 2);
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;
        SubscriptionQos subscriptionQos = new PeriodicSubscriptionQos(period_ms, expiryDate_ms, 0, 0);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQos);
        Thread.sleep(subscriptionDuration);
        verify(integerListener, times(0)).publicationMissed();
        verify(integerListener, atLeast(2)).receive(anyInt());

        reset(integerListener);
        Thread.sleep(subscriptionDuration);
        verifyNoMoreInteractions(integerListener);
        proxy.unsubscribeFromTestAttribute(subscriptionId);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void testOnChangeWithKeepAliveSubscriptionSendsOnChange() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        // NOTE: 50 is the minimum minInterval supported
        long minInterval_ms = 50;
        long subscriptionDuration = 1000;
        // publications don't live longer than subscription
        long publicationTtl_ms = subscriptionDuration;
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;

        // do not want to see the interval
        long maxInterval_ms = subscriptionDuration + 1;
        SubscriptionQos subscriptionQosMixed = new OnChangeWithKeepAliveSubscriptionQos(minInterval_ms,
                                                                                        maxInterval_ms,
                                                                                        expiryDate_ms,
                                                                                        publicationTtl_ms);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQosMixed);
        verify(integerListener, times(0)).publicationMissed();
        Thread.sleep(expected_latency_ms);

        // when subscribing, we automatically get 1 publication. Expect the starting-publication
        verify(integerListener, times(1)).receive(anyInt());

        // Wait minimum time between onChanged
        Thread.sleep(minInterval_ms);
        provider.setTestAttribute(5);
        Thread.sleep(expected_latency_ms);
        // expect the onChangeSubscription to have arrived
        verify(integerListener, times(2)).receive(anyInt());

        Thread.sleep(subscriptionDuration);
        // expect no more publications to arrive
        verifyNoMoreInteractions(integerListener);

        proxy.unsubscribeFromTestAttribute(subscriptionId);

    }

    @SuppressWarnings("unchecked")
    @Test
    public void testOnChangeWithKeepAliveSubscriptionSendsKeepAlive() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        // NOTE: 50 is the minimum minInterval supported
        long minInterval_ms = 50;
        // get an interval update after 200 ms
        long maxInterval_ms = 200;
        int numberExpectedKeepAlives = 3;
        // the subscription duration is a little longer so that it does not expire exactly as the last keep alive is to be sent
        long subscriptionDuration = maxInterval_ms * numberExpectedKeepAlives + (maxInterval_ms / 2);
        long publicationTtl_ms = maxInterval_ms; // publications don't live
        // longer than next interval
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;

        SubscriptionQos subscriptionQosMixed = new OnChangeWithKeepAliveSubscriptionQos(minInterval_ms,
                                                                                        maxInterval_ms,
                                                                                        expiryDate_ms,
                                                                                        0,
                                                                                        publicationTtl_ms);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQosMixed);
        verify(integerListener, times(0)).publicationMissed();
        Thread.sleep(expected_latency_ms);

        // when subscribing, we automatically get 1 publication. Expect the
        // starting-publication
        verify(integerListener, times(1)).receive(anyInt());

        for (int i = 1; i <= numberExpectedKeepAlives; i++) {

            Thread.sleep(maxInterval_ms + expected_latency_ms);
            // expect the next keep alive notification to have now arrived (plus the original one at subscription start)
            verify(integerListener, times(i + 1)).receive(anyInt());
        }

        proxy.unsubscribeFromTestAttribute(subscriptionId);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void testOnChangeWithKeepAliveSubscription() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        long minInterval_ms = 50; // NOTE: 50 is the minimum minInterval
        // supported
        long maxInterval_ms = 500; // get an interval update after 200 ms
        long subscriptionDuration = maxInterval_ms * 3;
        long alertInterval_ms = maxInterval_ms + 100;
        long publicationTtl_ms = maxInterval_ms; // publications don't live
        // longer than next interval
        long expiryDate_ms = System.currentTimeMillis() + subscriptionDuration;

        SubscriptionQos subscriptionQosMixed = new OnChangeWithKeepAliveSubscriptionQos(minInterval_ms,
                                                                                        maxInterval_ms,
                                                                                        expiryDate_ms,
                                                                                        alertInterval_ms,
                                                                                        publicationTtl_ms);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQosMixed);
        verify(integerListener, times(0)).publicationMissed();
        Thread.sleep(expected_latency_ms);

        // when subscribing, we automatically get 1 publication. Expect the
        // starting-publication
        verify(integerListener, times(1)).receive(anyInt());

        // Wait minimum time between onChanged
        Thread.sleep(minInterval_ms);
        provider.setTestAttribute(5);
        Thread.sleep(expected_latency_ms);
        // expect the onChangeSubscription to have arrived
        verify(integerListener, times(2)).receive(anyInt());

        Thread.sleep(maxInterval_ms + 50);
        // expect a keep alive notification to have now arrived
        verify(integerListener, atLeast(3)).receive(anyInt());

        proxy.unsubscribeFromTestAttribute(subscriptionId);

    }

    @SuppressWarnings("unchecked")
    @Test
    public void testOnChangeSubscription() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        long minInterval_ms = 0;
        long publicationTtl_ms = 1000;
        long expiryDate_ms = System.currentTimeMillis() + 1000;

        SubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval_ms, expiryDate_ms, publicationTtl_ms);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQos);
        Thread.sleep(expected_latency_ms);
        verify(integerListener, times(0)).publicationMissed();
        // when subscribing, we automatically get 1 publication. This might not
        // be the case in java?
        verify(integerListener, times(1)).receive(anyInt());

        provider.setTestAttribute(5);
        Thread.sleep(expected_latency_ms);

        verify(integerListener, times(2)).receive(anyInt());

        proxy.unsubscribeFromTestAttribute(subscriptionId);

    }

    @SuppressWarnings("unchecked")
    @Test
    public void testExpiredOnChangeSubscription() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);

        // Only get onChange messages
        long minInterval_ms = 0;
        long duration = 500;
        // Expire quickly
        long expiryDate_ms = System.currentTimeMillis() + duration;
        // Have a large TTL on subscription messages
        long publicationTtl_ms = 10000;

        SubscriptionQos subscriptionQos = new OnChangeSubscriptionQos(minInterval_ms, expiryDate_ms, publicationTtl_ms);

        String subscriptionId = proxy.subscribeToTestAttribute(integerListener, subscriptionQos);
        Thread.sleep(expected_latency_ms);
        // There should have only been one call - the automatic publication when a subscription is made
        verify(integerListener, times(1)).receive(anyInt());

        Thread.sleep(duration + expected_latency_ms);
        // We should now have an expired onChange subscription
        provider.setTestAttribute(5);
        Thread.sleep(100);
        verifyNoMoreInteractions(integerListener);

        proxy.unsubscribeFromTestAttribute(subscriptionId);

    }

    @edu.umd.cs.findbugs.annotations.SuppressWarnings(value = "NP_NULL_ON_SOME_PATH_EXCEPTION", justification = "NPE in test would fail test")
    @SuppressWarnings("unchecked")
    @Test
    public void testSubscribeToNonExistentDomain() throws InterruptedException {
        SubscriptionListener<Integer> integerListener = mock(SubscriptionListener.class);
        TestProxy proxyToNonexistentDomain = null;
        try {
            ProxyBuilder<TestProxy> proxyBuilder;
            String nonExistentDomain = UUID.randomUUID().toString() + "-domaindoesnotexist-end2end";
            MessagingQos messagingQos = new MessagingQos(20000);
            DiscoveryQos discoveryQos = new DiscoveryQos(50000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);
            proxyBuilder = consumingApplication.getRuntime().getProxyBuilder(nonExistentDomain,
                                                                             joynr.tests.TestProxy.class);
            proxyToNonexistentDomain = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        } catch (JoynrArbitrationException e) {
            e.printStackTrace();
        } catch (JoynrIllegalStateException e) {
            e.printStackTrace();
        }

        // This should not cause an exception
        SubscriptionQos subscriptionQos = new PeriodicSubscriptionQos(period_ms,
                                                                      System.currentTimeMillis() + 30000,
                                                                      0,
                                                                      0);

        String subscriptionId = proxyToNonexistentDomain.subscribeToTestAttribute(integerListener, subscriptionQos);
        Thread.sleep(4000);
        proxyToNonexistentDomain.unsubscribeFromTestAttribute(subscriptionId);
    }
}
