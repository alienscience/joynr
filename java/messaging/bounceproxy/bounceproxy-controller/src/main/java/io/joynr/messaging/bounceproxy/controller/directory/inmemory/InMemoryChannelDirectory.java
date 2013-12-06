package io.joynr.messaging.bounceproxy.controller.directory.inmemory;

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

import io.joynr.messaging.bounceproxy.controller.directory.ChannelDirectory;
import io.joynr.messaging.info.ChannelInformation;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import com.google.inject.Singleton;

/**
 * @author christina.strobel
 *
 */
@Singleton
public class InMemoryChannelDirectory implements ChannelDirectory {

    private HashMap<String, ChannelInformation> channels = new HashMap<String, ChannelInformation>();

    @Override
    public List<ChannelInformation> getChannels() {
        return new LinkedList<ChannelInformation>(channels.values());
    }

    @Override
    public ChannelInformation getChannel(String ccid) {
        return channels.get(ccid);
    }

    @Override
    public void addChannel(ChannelInformation channelInfo) {
        channels.put(channelInfo.getChannelId(), channelInfo);
    }

}