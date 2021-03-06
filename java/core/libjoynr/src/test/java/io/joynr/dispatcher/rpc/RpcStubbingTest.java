package io.joynr.dispatcher.rpc;

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

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import io.joynr.common.JoynrPropertiesModule;
import io.joynr.dispatcher.DispatcherTestModule;
import io.joynr.dispatcher.RequestCaller;
import io.joynr.dispatcher.RequestReplyDispatcher;
import io.joynr.dispatcher.RequestReplySender;
import io.joynr.dispatcher.SynchronizedReplyCaller;
import io.joynr.dispatcher.rpc.annotation.JoynrRpcParam;
import io.joynr.endpoints.EndpointAddressBase;
import io.joynr.endpoints.JoynrMessagingEndpointAddress;
import io.joynr.exceptions.JoynrCommunicationException;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.messaging.MessagingModule;
import io.joynr.messaging.MessagingQos;
import io.joynr.provider.JoynrProvider;
import io.joynr.provider.RequestCallerFactory;
import io.joynr.runtime.PropertyLoader;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

import joynr.Reply;
import joynr.Request;
import joynr.types.GpsFixEnum;
import joynr.types.GpsLocation;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.runners.MockitoJUnitRunner;
import org.mockito.stubbing.Answer;

import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.google.inject.Guice;
import com.google.inject.Injector;

/**
 * Simulates consumer-side call to JoynMessagingConnectorInvocationHandler, with the request being
 * 
 * @author david.katz
 * 
 */
@RunWith(MockitoJUnitRunner.class)
public class RpcStubbingTest {
    private static final GpsLocation gpsValue = new GpsLocation(1.0d,
                                                                2.0d,
                                                                0d,
                                                                GpsFixEnum.MODE2D,
                                                                0d,
                                                                0d,
                                                                0d,
                                                                0d,
                                                                0l,
                                                                0l,
                                                                0);

    private static final List<GpsLocation> gpsList = Arrays.asList(new GpsLocation[]{ gpsValue,
            new GpsLocation(3.0d, 4.0d, 0d, GpsFixEnum.MODE2D, 0d, 0d, 0d, 0d, 0l, 0l, 0) });

    private static final long DEFAULT_TTL = 2000L;

    public static interface TestSyncInterface extends JoynrSyncInterface {
        public GpsLocation returnsGpsLocation();

        public List<GpsLocation> returnsGpsLocationList();

        public void takesTwoSimpleParams(@JoynrRpcParam("a") Integer a, @JoynrRpcParam("b") String b);

        public void noParamsNoReturnValue();
    }

    public static interface TestAsyncInterface extends JoynrAsyncInterface {

    }

    public static interface TestInterface extends JoynrInterface, TestSyncInterface, TestAsyncInterface {

    }

    public interface TestProviderInterface extends TestInterface, JoynrProvider {

    }

    @Mock
    private RequestReplyDispatcher dispatcher;

    @Mock
    private RequestReplySender messageSender;

    @Mock
    private TestProviderInterface testMock;

    // private String domain;
    // private String interfaceName;
    private String fromParticipantId;
    private String toParticipantId;

    private Injector injector;

    private JoynrMessagingConnectorInvocationHandler connector;

    private static final JoynrMessagingEndpointAddress endpointAddress = new JoynrMessagingEndpointAddress("channelId");

    @Before
    @edu.umd.cs.findbugs.annotations.SuppressWarnings(value = "NP_NULL_PARAM_DEREF", justification = "NPE in test would fail test")
    public void setUp() throws JoynrCommunicationException, JoynrSendBufferFullException, JsonGenerationException,
                       JsonMappingException, IOException, JoynrMessageNotSentException {

        when(testMock.returnsGpsLocation()).thenReturn(gpsValue);
        when(testMock.returnsGpsLocationList()).thenReturn(gpsList);

        fromParticipantId = UUID.randomUUID().toString();
        toParticipantId = UUID.randomUUID().toString();

        // required to inject static members of JoynMessagingConnectorFactory
        injector = Guice.createInjector(new MessagingModule(),
                                        new JoynrPropertiesModule(PropertyLoader.loadProperties("defaultMessaging.properties")),
                                        new DispatcherTestModule());

        final JsonRequestInterpreter jsonRequestInterpreter = injector.getInstance(JsonRequestInterpreter.class);
        final RequestCallerFactory requestCallerFactory = injector.getInstance(RequestCallerFactory.class);

        when(messageSender.sendSyncRequest(eq(fromParticipantId),
                                           eq(toParticipantId),
                                           any(EndpointAddressBase.class),
                                           any(Request.class),
                                           any(SynchronizedReplyCaller.class),
                                           eq(DEFAULT_TTL))).thenAnswer(new Answer<Reply>() {

            private RequestCaller requestCaller = requestCallerFactory.create(testMock, TestSyncInterface.class);

            @Override
            public Reply answer(InvocationOnMock invocation) throws Throwable {
                Object[] args = invocation.getArguments();
                Request request = null;
                for (Object arg : args) {
                    if (arg instanceof Request) {
                        request = (Request) arg;
                        break;
                    }
                }

                return jsonRequestInterpreter.execute(requestCaller, request);
            }
        });

        MessagingQos qosSettings = new MessagingQos(DEFAULT_TTL);
        connector = JoynrMessagingConnectorFactory.create(dispatcher,
                                                          messageSender,
                                                          fromParticipantId,
                                                          toParticipantId,
                                                          endpointAddress,
                                                          qosSettings);

    }

    @Test
    public void testWithoutArguments() throws IOException, JoynrSendBufferFullException, JoynrCommunicationException,
                                      JoynrMessageNotSentException, SecurityException, InstantiationException,
                                      IllegalAccessException, NoSuchMethodException {
        // Send
        String methodName = "noParamsNoReturnValue";
        Object result = connector.executeSyncMethod(TestSyncInterface.class.getDeclaredMethod(methodName,
                                                                                              new Class<?>[]{}),
                                                    new Object[]{});

        assertNull(result);

        ArgumentCaptor<Request> requestCaptor = ArgumentCaptor.forClass(Request.class);
        verify(messageSender).sendSyncRequest(eq(fromParticipantId),
                                              eq(toParticipantId),
                                              any(EndpointAddressBase.class),
                                              requestCaptor.capture(),
                                              any(SynchronizedReplyCaller.class),
                                              eq(DEFAULT_TTL));

        verify(testMock).noParamsNoReturnValue();
        assertEquals(methodName, requestCaptor.getValue().getMethodName());
        assertEquals(0, requestCaptor.getValue().getParamDatatypes().length);
    }

    @Test
    public void testWithArguments() throws IOException, JoynrSendBufferFullException, JoynrCommunicationException,
                                   JoynrMessageNotSentException, SecurityException, InstantiationException,
                                   IllegalAccessException, NoSuchMethodException {
        // Send

        String methodName = "takesTwoSimpleParams";
        Object[] args = new Object[]{ 3, "abc" };
        Object response = connector.executeSyncMethod(TestSyncInterface.class.getDeclaredMethod(methodName,
                                                                                                Integer.class,
                                                                                                String.class), args);

        // no return value expected
        assertNull(response);
        ArgumentCaptor<Request> requestCaptor = ArgumentCaptor.forClass(Request.class);
        verify(messageSender).sendSyncRequest(eq(fromParticipantId),
                                              eq(toParticipantId),
                                              any(EndpointAddressBase.class),
                                              requestCaptor.capture(),
                                              any(SynchronizedReplyCaller.class),
                                              eq(DEFAULT_TTL));

        assertEquals(methodName, requestCaptor.getValue().getMethodName());
        assertEquals(2, requestCaptor.getValue().getParamDatatypes().length);
        assertArrayEquals(args, requestCaptor.getValue().getParams());
        verify(testMock).takesTwoSimpleParams(3, "abc");

    }

    @Test
    public void testWithReturn() throws IOException, JoynrCommunicationException, JoynrSendBufferFullException,
                                JoynrMessageNotSentException, SecurityException, InstantiationException,
                                IllegalAccessException, NoSuchMethodException {
        // Send
        String methodName = "returnsGpsLocation";
        Object response = connector.executeSyncMethod(TestSyncInterface.class.getDeclaredMethod(methodName),
                                                      new Object[]{});

        ArgumentCaptor<Request> requestCaptor = ArgumentCaptor.forClass(Request.class);
        verify(messageSender).sendSyncRequest(eq(fromParticipantId),
                                              eq(toParticipantId),
                                              any(EndpointAddressBase.class),
                                              requestCaptor.capture(),
                                              any(SynchronizedReplyCaller.class),
                                              eq(DEFAULT_TTL));

        assertEquals(methodName, requestCaptor.getValue().getMethodName());
        assertEquals(0, requestCaptor.getValue().getParamDatatypes().length);
        verify(testMock).returnsGpsLocation();
        assertEquals(gpsValue, response);
    }

    @Test
    public void testWithListReturn() throws IOException, JoynrCommunicationException, JoynrSendBufferFullException,
                                    JoynrMessageNotSentException, SecurityException, InstantiationException,
                                    IllegalAccessException, NoSuchMethodException {
        // Send
        String methodName = "returnsGpsLocationList";
        Object response = connector.executeSyncMethod(TestSyncInterface.class.getDeclaredMethod(methodName),
                                                      new Object[]{});

        ArgumentCaptor<Request> requestCaptor = ArgumentCaptor.forClass(Request.class);
        verify(messageSender).sendSyncRequest(eq(fromParticipantId),
                                              eq(toParticipantId),
                                              any(EndpointAddressBase.class),
                                              requestCaptor.capture(),
                                              any(SynchronizedReplyCaller.class),
                                              eq(DEFAULT_TTL));

        assertEquals(methodName, requestCaptor.getValue().getMethodName());
        assertEquals(0, requestCaptor.getValue().getParamDatatypes().length);
        verify(testMock).returnsGpsLocationList();
        assertEquals(gpsList, response);
    }

}
