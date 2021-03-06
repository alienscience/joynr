package io.joynr.proxy;

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

import static org.mockito.Matchers.anyLong;
import static org.mockito.Matchers.anyString;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import io.joynr.arbitration.ArbitrationStrategy;
import io.joynr.arbitration.DiscoveryQos;
import io.joynr.capabilities.CapabilitiesCallback;
import io.joynr.capabilities.CapabilityEntry;
import io.joynr.capabilities.CapabilityScope;
import io.joynr.capabilities.LocalCapabilitiesDirectory;
import io.joynr.dispatcher.ReplyCaller;
import io.joynr.dispatcher.RequestReplyDispatcher;
import io.joynr.dispatcher.RequestReplySender;
import io.joynr.dispatcher.SynchronizedReplyCaller;
import io.joynr.dispatcher.rpc.Callback;
import io.joynr.dispatcher.rpc.JoynrAsyncInterface;
import io.joynr.dispatcher.rpc.JoynrSyncInterface;
import io.joynr.dispatcher.rpc.RequestStatusCode;
import io.joynr.dispatcher.rpc.RpcUtils;
import io.joynr.dispatcher.rpc.annotation.JoynrRpcCallback;
import io.joynr.endpoints.EndpointAddressBase;
import io.joynr.endpoints.JoynrMessagingEndpointAddress;
import io.joynr.exceptions.JoynrArbitrationException;
import io.joynr.exceptions.JoynrCommunicationException;
import io.joynr.exceptions.JoynrIllegalStateException;
import io.joynr.messaging.MessagingQos;
import io.joynr.pubsub.subscription.SubscriptionManager;

import java.io.IOException;
import java.util.ArrayList;

import joynr.Reply;
import joynr.Request;
import joynr.types.ProviderQos;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.runners.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;

import com.fasterxml.jackson.core.JsonParseException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.node.TextNode;
import com.google.inject.AbstractModule;
import com.google.inject.Guice;

@RunWith(MockitoJUnitRunner.class)
public class ProxyTest {
    private ProxyBuilder<TestInterface> proxyBuilder;
    private DiscoveryQos discoveryQos;
    private MessagingQos messagingQos;
    @Mock
    private RequestReplyDispatcher dispatcher1;
    @Mock
    private RequestReplySender joynrMessageSender1;
    @Mock
    SubscriptionManager subscriptionManager1;

    @Mock
    private LocalCapabilitiesDirectory capabilitiesClient;

    private String domain;
    private String asyncReplyText = "replyText";

    @Mock
    private Callback<String> callback;

    public interface SyncTestInterface extends JoynrSyncInterface {
        String method1();
    }

    public static class StringTypeRef extends TypeReference<String> {
    }

    public interface AsyncTestInterface extends JoynrAsyncInterface {
        Future<String> asyncMethod(@JoynrRpcCallback(deserialisationType = StringTypeRef.class) Callback<String> callback);
    }

    public interface TestInterface extends SyncTestInterface, AsyncTestInterface {
        public static final String INTERFACE_NAME = "TestInterface";
    }

    @Before
    public void setUp() throws Exception {
        Guice.createInjector(new AbstractModule() {

            @Override
            protected void configure() {
                requestStaticInjection(RpcUtils.class);
            }

        });

        Mockito.doAnswer(new Answer<Object>() {
            public Object answer(InvocationOnMock invocation) {
                Object[] args = invocation.getArguments();
                ArrayList<CapabilityEntry> fakeCapabilitiesResult = new ArrayList<CapabilityEntry>();
                fakeCapabilitiesResult.add(new CapabilityEntry(domain,
                                                               TestInterface.class,
                                                               new ProviderQos(),
                                                               new JoynrMessagingEndpointAddress("testChannelId"),
                                                               "TestParticipantId",
                                                               CapabilityScope.LOCALONLY));
                ((CapabilitiesCallback) args[3]).processCapabilitiesReceived(fakeCapabilitiesResult);
                return null;
            }
        }).when(capabilitiesClient).getCapabilities(Mockito.<String> any(),
                                                    Mockito.<String> any(),
                                                    Mockito.<DiscoveryQos> any(),
                                                    Mockito.<CapabilitiesCallback> any());

        domain = "TestDomain";
        proxyBuilder = new ProxyBuilderDefaultImpl<TestInterface>(capabilitiesClient,
                                                                  domain,
                                                                  TestInterface.class,
                                                                  joynrMessageSender1,
                                                                  dispatcher1,
                                                                  subscriptionManager1);
        discoveryQos = new DiscoveryQos(10000, ArbitrationStrategy.HighestPriority, Long.MAX_VALUE);
        messagingQos = new MessagingQos();
    }

    @Test
    public void createProxyAndCallSyncMethod() throws Exception {
        String requestReplyId = "createProxyAndCallSyncMethod_requestReplyId";
        Mockito.when(joynrMessageSender1.sendSyncRequest(Mockito.<String> any(),
                                                         Mockito.<String> any(),
                                                         Mockito.<EndpointAddressBase> any(),
                                                         Mockito.<Request> any(),
                                                         Mockito.<SynchronizedReplyCaller> any(),
                                                         Mockito.anyLong())).thenReturn(new Reply(requestReplyId,
                                                                                                  "Answer"));

        TestInterface proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();
        String result = proxy.method1();
        Assert.assertEquals("Answer", result);

    }

    @Test
    public void createProxyAndCallAsyncMethodSuccess() throws Exception {

        try {
            TestInterface proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

            // when joynrMessageSender1.sendRequest is called, get the replyCaller from the mock dispatcher and call
            // messageCallback on it.
            Mockito.doAnswer(new Answer<Object>() {
                public Object answer(InvocationOnMock invocation) throws JsonParseException, JsonMappingException,
                                                                 IOException {
                    // capture the replyCaller passed into the dispatcher for calling later
                    ArgumentCaptor<ReplyCaller> replyCallerCaptor = ArgumentCaptor.forClass(ReplyCaller.class);
                    verify(dispatcher1).addReplyCaller(anyString(), replyCallerCaptor.capture(), anyLong());

                    String requestReplyId = "createProxyAndCallAsyncMethodSuccess_requestReplyId";
                    // pass the response to the replyCaller
                    replyCallerCaptor.getValue()
                                     .messageCallBack(new Reply(requestReplyId, new TextNode(asyncReplyText)));
                    return null;
                }
            }).when(joynrMessageSender1).sendRequest(Mockito.<String> any(),
                                                     Mockito.<String> any(),
                                                     Mockito.<EndpointAddressBase> any(),
                                                     Mockito.<Request> any(),
                                                     Mockito.anyLong());
            final Future<String> future = proxy.asyncMethod(callback);

            // the test usually takes only 200 ms, so if we wait 1 sec, something has gone wrong
            String reply = future.getReply(1000);

            verify(callback).onSuccess(asyncReplyText);
            Assert.assertEquals(RequestStatusCode.OK, future.getStatus().getCode());
            Assert.assertEquals(asyncReplyText, reply);

        } catch (JoynrArbitrationException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (JoynrIllegalStateException e) {
            e.printStackTrace();
        }

    }

    @Test
    public void createProxyAndCallAsyncMethodFail() throws Exception {

        // Expect this exception to be passed back to the callback onFailure and thrown in the future
        final JoynrCommunicationException expectedException = new JoynrCommunicationException();
        // final JoynCommunicationException expectedException = null;

        TestInterface proxy = proxyBuilder.setMessagingQos(messagingQos).setDiscoveryQos(discoveryQos).build();

        // when joynrMessageSender1.sendRequest is called, get the replyCaller from the mock dispatcher and call
        // messageCallback on it.
        Mockito.doAnswer(new Answer<Object>() {
            public Object answer(InvocationOnMock invocation) throws JsonParseException, JsonMappingException,
                                                             IOException {
                // capture the replyCaller passed into the dispatcher for calling later
                ArgumentCaptor<ReplyCaller> replyCallerCaptor = ArgumentCaptor.forClass(ReplyCaller.class);
                verify(dispatcher1).addReplyCaller(anyString(), replyCallerCaptor.capture(), anyLong());

                // pass the exception to the replyCaller
                replyCallerCaptor.getValue().error(expectedException);
                return null;
            }
        }).when(joynrMessageSender1).sendRequest(Mockito.<String> any(),
                                                 Mockito.<String> any(),
                                                 Mockito.<EndpointAddressBase> any(),
                                                 Mockito.<Request> any(),
                                                 Mockito.anyLong());

        boolean exceptionThrown = false;
        String reply = "";
        final Future<String> future = proxy.asyncMethod(callback);
        try {
            // the test usually takes only 200 ms, so if we wait 1 sec, something has gone wrong
            reply = future.getReply(1000);
        } catch (JoynrCommunicationException e) {
            exceptionThrown = true;
        }
        Assert.assertTrue("exception must be thrown from getReply", exceptionThrown);
        verify(callback).onFailure(expectedException);
        verifyNoMoreInteractions(callback);
        Assert.assertEquals(RequestStatusCode.ERROR, future.getStatus().getCode());
        Assert.assertEquals("", reply);
    }
}
