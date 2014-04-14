package io.joynr.integration;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Properties;
import java.util.UUID;

import joynr.tests.DefaultTestProvider;
import joynr.tests.TestProxy;
import io.joynr.arbitration.ArbitrationStrategy;
import io.joynr.arbitration.DiscoveryQos;
import io.joynr.integration.ProviderProxyEnd2EndTest.TestProvider;
import io.joynr.integration.util.DummyJoynrApplication;
import io.joynr.integration.util.SSLSettings;
import io.joynr.integration.util.ServersUtil;
import io.joynr.messaging.ConfigurableMessagingSettings;
import io.joynr.messaging.MessagingPropertyKeys;
import io.joynr.messaging.MessagingQos;
import io.joynr.proxy.ProxyBuilder;
import io.joynr.runtime.AbstractJoynrApplication;
import io.joynr.runtime.JoynrInjectorFactory;
import io.joynr.runtime.PropertyLoader;

import org.eclipse.jetty.server.Server;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
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
public class SSLEnd2EndTest {

    private static final Logger logger = LoggerFactory.getLogger(SSLEnd2EndTest.class);

    private static Server jettyServer;
    private static String resourcePath;

    private DummyJoynrApplication dummyProviderApplication;
    private DummyJoynrApplication dummyConsumerApplication;

    DefaultTestProvider provider;
    String domain;

    private MessagingQos messagingQos;
    private DiscoveryQos discoveryQos;

    @Rule
    public TestName name = new TestName();

    @BeforeClass
    public static void startServer() throws Exception {

        resourcePath = getResourcePath();

        // Set global SSL properties for all Joynr SSL clients
        System.setProperty("javax.net.ssl.keyStore", resourcePath + "/javaclient.jks");
        System.setProperty("javax.net.ssl.trustStore", resourcePath + "/truststore.jks");
        System.setProperty("javax.net.ssl.keyStorePassword", "changeit");
        System.setProperty("javax.net.ssl.trustStorePassword", "changeit");

        // Uncomment the line below to enable SSL debug
        // System.setProperty("javax.net.debug", "ssl");

        // Set Jetty SSL properties for bounce proxy and discovery directory servlet listeners
        logger.debug("resource path : {}", resourcePath);
        SSLSettings settings = new SSLSettings(resourcePath + "/localhost.jks", // KeyStore
                                               resourcePath + "/truststore.jks", // TrustStore
                                               "changeit", // KeyStore password
                                               "changeit" // TrustStore password
        );

        jettyServer = ServersUtil.startSSLServers(settings);

        // keep delays and timeout low for tests
        System.setProperty(ConfigurableMessagingSettings.PROPERTY_SEND_MSG_RETRY_INTERVAL_MS, "10");
        System.setProperty(ConfigurableMessagingSettings.PROPERTY_DISCOVERY_REQUEST_TIMEOUT, "1000");
    }

    @AfterClass
    public static void stopServer() throws Exception {
        jettyServer.stop();
    }

    // Get the path of the test resources
    private static String getResourcePath() throws URISyntaxException {
        URL resource = ClassLoader.getSystemClassLoader().getResource("truststore.jks");
        File fullPath = new File(resource.toURI());
        return fullPath.getParent();
    }

    @Before
    public void setup() throws InterruptedException {

        String methodName = name.getMethodName();
        logger.info("{} setup beginning...", methodName);

        // use channelNames = test name
        String channelIdProvider = "JavaTest-" + methodName + UUID.randomUUID().getLeastSignificantBits()
                + "-end2endTestProvider";
        String channelIdConsumer = "JavaTest-" + methodName + UUID.randomUUID().getLeastSignificantBits()
                + "-end2endConsumer";

        Properties joynrConfigProvider = PropertyLoader.loadProperties("testMessaging.properties");
        joynrConfigProvider.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain."
                + UUID.randomUUID().toString());
        joynrConfigProvider.put(MessagingPropertyKeys.CHANNELID, channelIdProvider);
        joynrConfigProvider.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());

        dummyProviderApplication = (DummyJoynrApplication) new JoynrInjectorFactory(joynrConfigProvider).createApplication(DummyJoynrApplication.class);

        Properties joynrConfigConsumer = PropertyLoader.loadProperties("testMessaging.properties");
        joynrConfigConsumer.put(AbstractJoynrApplication.PROPERTY_JOYNR_DOMAIN_LOCAL, "localdomain."
                + UUID.randomUUID().toString());
        joynrConfigConsumer.put(MessagingPropertyKeys.CHANNELID, channelIdConsumer);
        joynrConfigConsumer.put(MessagingPropertyKeys.RECEIVERID, UUID.randomUUID().toString());

        dummyConsumerApplication = (DummyJoynrApplication) new JoynrInjectorFactory(joynrConfigConsumer).createApplication(DummyJoynrApplication.class);

        provider = new DefaultTestProvider();
        domain = "SSLEnd2EndTest." + methodName + System.currentTimeMillis();

        dummyProviderApplication.getRuntime().registerCapability(domain,
                                                                 provider,
                                                                 joynr.tests.TestSync.class,
                                                                 "authToken");

        messagingQos = new MessagingQos(5000);
        discoveryQos = new DiscoveryQos(5000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);

        // Make sure the channel is created before the first messages sent.
        Thread.sleep(200);
        logger.info("setup finished");
    }

    @After
    public void tearDown() {
        if (dummyProviderApplication != null) {
            dummyProviderApplication.shutdown();
        }
        if (dummyConsumerApplication != null) {
            dummyConsumerApplication.shutdown();
        }
    }

    @Test
    public void getAndSetAttribute() {

        // Build a client proxy
        ProxyBuilder<TestProxy> proxyBuilder = dummyConsumerApplication.getRuntime().getProxyBuilder(domain,
                                                                                                     TestProxy.class);

        TestProxy proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        // Set an attribute value
        int value = 1234;
        proxy.setReadWriteAttribute(value);

        // Get the attribute value
        int actual = proxy.getReadWriteAttribute();
        Assert.assertEquals(value, actual);
    }

}
