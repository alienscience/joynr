package io.joynr.messaging.bounceproxy.controller;

/*
 * #%L
 * joynr::java::messaging::bounceproxy::bounceproxy-controller
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

/**
 * Properties to configure the bounce proxy controller in properties files.
 * 
 * @author christina.strobel
 * 
 */
public class BounceProxyControllerPropertyKeys {

    public static final String PROPERTY_BPC_SEND_CREATE_CHANNEL_RETRY_INTERVAL_MS = "joynr.bounceproxy.controller.send_create_channel_retry_interval_ms";
    public static final String PROPERTY_BPC_SEND_CREATE_CHANNEL_MAX_RETRY_COUNT = "joynr.bounceproxy.controller.send_create_channel_max_retries";

}