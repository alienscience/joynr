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

import io.joynr.integration.util.ServersUtil;
import io.joynr.messaging.ConfigurableMessagingSettings;

import org.eclipse.jetty.server.Server;
import org.junit.AfterClass;
import org.junit.BeforeClass;

public class ControlledBounceProxyMessagingIntegrationTest extends AbstractMessagingIntegrationTest {

    private static Server bounceProxyServerXY;
    private static Server bounceProxyControllerServer;

    @BeforeClass
    public static void startServer() throws Exception {
        bounceProxyControllerServer = ServersUtil.startBounceproxyController();
        bounceProxyServerXY = ServersUtil.startControlledBounceproxy("X.Y");
        System.setProperty(ConfigurableMessagingSettings.PROPERTY_SEND_MSG_RETRY_INTERVAL_MS, "10");
        System.setProperty(ConfigurableMessagingSettings.PROPERTY_DISCOVERY_REQUEST_TIMEOUT, "200");
    }

    @AfterClass
    public static void stopServer() throws Exception {
        bounceProxyServerXY.stop();
        bounceProxyControllerServer.stop();
    }

}
