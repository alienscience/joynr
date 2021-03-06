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

import java.util.concurrent.Future;

import javax.annotation.CheckForNull;

import joynr.JoynrMessage;

/**
 * Messaging facade.
 */
public interface MessageReceiver {

    String getChannelId();

    void registerMessageListener(MessageArrivedListener messageReceiver);

    /**
     * 
     * @param clear
     *            indicates whether the messageListener should be dropped and the channel closed
     */
    void shutdown(boolean clear);

    boolean deleteChannel();

    boolean isStarted();

    void receive(JoynrMessage message);

    void onError(@CheckForNull JoynrMessage message, Throwable error);

    void suspend();

    void resume();

    boolean isChannelCreated();

    /**
     * @returns a future that signals when the receiver is ready to be used.  
     */
    Future<Void> startReceiver(ReceiverStatusListener... receiverStatusListeners);
}
