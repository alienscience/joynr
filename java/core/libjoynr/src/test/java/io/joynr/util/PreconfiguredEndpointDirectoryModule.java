package io.joynr.util;

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

import io.joynr.dispatcher.MessagingEndpointDirectory;
import io.joynr.messaging.IMessageReceivers;
import io.joynr.messaging.MessageReceivers;

import com.google.inject.AbstractModule;
import com.google.inject.Provides;

public class PreconfiguredEndpointDirectoryModule extends AbstractModule {
    MessagingEndpointDirectory messagingEndpointDirectory;

    public PreconfiguredEndpointDirectoryModule(MessagingEndpointDirectory messagingEndpointDirectory) {
        this.messagingEndpointDirectory = messagingEndpointDirectory;
    }

    @Provides
    MessagingEndpointDirectory provideEndpointDirectory() {
        return messagingEndpointDirectory;
    }

    @Override
    protected void configure() {
        bind(IMessageReceivers.class).to(MessageReceivers.class).asEagerSingleton();
    }
}
