package io.joynr.capabilities.directory;

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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import io.joynr.capabilities.CapabilitiesStoreImpl;

import java.util.ArrayList;
import java.util.UUID;

import joynr.types.CapabilityInformation;
import joynr.types.ProviderQos;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class CapabilitiesDirectoryTest {

    CapabilitiesDirectoryImpl capabilitiesDirectory;
    ArrayList<CapabilityInformation> singleInterface = new ArrayList<CapabilityInformation>();
    ArrayList<CapabilityInformation> multipleInterfaces = new ArrayList<CapabilityInformation>();

    String mcId = "capabilitiesProvider";
    String domain = "com";
    String thisInterface = "registerThisInterface";
    String anotherInterface = "registerAnotherInterface";
    String participantId1 = "testParticipantId1";
    String participantId2 = "testParticipantId2";
    ProviderQos providerQos = new ProviderQos();
    CapabilityInformation capInfo1;
    CapabilityInformation capInfo2;
    String postFix = "" + System.currentTimeMillis();

    @Before
    public void setUp() {
        capabilitiesDirectory = new CapabilitiesDirectoryImpl(new CapabilitiesStoreImpl());
        providerQos.setPriority((long) 123);
        capInfo1 = new CapabilityInformation(domain, thisInterface, providerQos, mcId, participantId1);
        capInfo2 = new CapabilityInformation(domain, anotherInterface, providerQos, mcId, participantId2);
        singleInterface.add(capInfo1);
        multipleInterfaces.add(capInfo1);
        multipleInterfaces.add(capInfo2);
    }

    @After
    public void tearDown() {
    }

    @Test
    public void registerMultipleCapabilities() {

        capabilitiesDirectory.registerCapabilities(singleInterface);
        capabilitiesDirectory.registerCapabilities(multipleInterfaces);

        assertEquals(multipleInterfaces.get(0), capabilitiesDirectory.lookupCapabilities(domain, thisInterface).get(0));
        assertTrue(capabilitiesDirectory.lookupCapabilities(domain, thisInterface).contains(multipleInterfaces.get(0)));
        assertEquals(multipleInterfaces.get(1), capabilitiesDirectory.lookupCapabilities(domain, anotherInterface)
                                                                     .get(0));
        assertTrue(capabilitiesDirectory.lookupCapabilities(domain, anotherInterface)
                                        .contains(multipleInterfaces.get(1)));
    }

    @Test
    public void registerCapabilityAndRequestChannels() throws Exception {
        capabilitiesDirectory.registerCapabilities(singleInterface);
        assertEquals(singleInterface.get(0), capabilitiesDirectory.lookupCapabilities(domain, thisInterface).get(0));
        assertEquals(true, capabilitiesDirectory.lookupCapabilities(domain, thisInterface)
                                                .contains(singleInterface.get(0)));

    }

    @Test
    public void registerCapabilityAndRequestCapabilites() throws Exception {
        capabilitiesDirectory.registerCapabilities(singleInterface);
        assertEquals(true, capabilitiesDirectory.getCapabilitiesForChannelId(mcId).contains(singleInterface.get(0)));
    }

    @Test
    public void registerDeleteAndRequestCapability() {

        capabilitiesDirectory.registerCapabilities(singleInterface);
        assertEquals(true, capabilitiesDirectory.getCapabilitiesForChannelId(mcId).contains(singleInterface.get(0)));
        capabilitiesDirectory.unregisterCapabilities(singleInterface);
        assertEquals(false, capabilitiesDirectory.getCapabilitiesForChannelId(mcId).contains(capInfo1));
    }

    String getRandomParticipantId() {
        return "participantId-" + String.valueOf(UUID.randomUUID().getLeastSignificantBits()).substring(4);
    }

    String getRandomChannelId() {
        return "channel-" + String.valueOf(UUID.randomUUID().getLeastSignificantBits()).substring(4);
    }

    String getRandomInterface(String prefix) {
        return prefix + "-" + "interface-" + String.valueOf(UUID.randomUUID().getLeastSignificantBits()).substring(4);
    }

    String getRandomDomain(String type) {
        return type + "." + "testdomains." + String.valueOf(UUID.randomUUID().getLeastSignificantBits()).substring(4);
    }

}
