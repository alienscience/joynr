package io.joynr.messaging;

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

import io.joynr.runtime.AbstractJoynrApplication;
import joynr.chat.DefaultMessengerProvider;
import joynr.chat.Message;
import joynr.chat.MessengerProvider;

public class ServletJoynrChatApplication extends AbstractJoynrApplication {

    // @Inject
    // @Named("DummyJoynApplication.participantId")
    // String participantId;

    private DefaultMessengerProvider provider;

    @Override
    public void run() {
        provider = new DefaultMessengerProvider() {
            @Override
            public void setMessage(Message message) {
                // manipulate the message so that the consumer can verify that the set worked
                message.setMessage(message.getSenderId() + message.getMessage());
                super.setMessage(message);
            }

        };
        runtime.registerCapability(localDomain, provider, MessengerProvider.class, "ServletJoynChatApplication");
    }

    @Override
    public void shutdown() {
        runtime.unregisterCapability(localDomain, provider, MessengerProvider.class, "ServletJoynChatApplication");

    }
}
