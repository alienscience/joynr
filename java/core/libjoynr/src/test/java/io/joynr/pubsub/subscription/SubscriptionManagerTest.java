package io.joynr.pubsub.subscription;

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

import io.joynr.pubsub.PubSubState;
import io.joynr.pubsub.SubscriptionQos;

import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import joynr.PeriodicSubscriptionQos;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

import com.fasterxml.jackson.core.type.TypeReference;

@RunWith(MockitoJUnitRunner.class)
public class SubscriptionManagerTest {

    private String attributeName;
    private SubscriptionListener<?> attributeSubscriptionCallback;

    private SubscriptionQos qos;

    @Mock
    private ConcurrentMap<String, SubscriptionListener<?>> attributeSubscriptionDirectory;

    @Mock
    private ConcurrentMap<String, PubSubState> subscriptionStates;

    @Mock
    ConcurrentMap<String, MissedPublicationTimer> missedPublicationTimers;

    @Mock
    ConcurrentMap<String, ScheduledFuture<?>> subscriptionEndFutures;

    @Mock
    private PubSubState subscriptionState;

    private SubscriptionManager subscriptionManager;
    private String subscriptionId;
    private MissedPublicationTimer missedPublicationTimer;

    @Mock
    private ConcurrentMap<String, Class<? extends TypeReference<?>>> subscriptionAttributeTypes;
    @Mock
    private ScheduledExecutorService cleanupScheduler;

    @Before
    public void setUp() {
        subscriptionManager = new SubscriptionManagerImpl(attributeSubscriptionDirectory,
                                                          subscriptionStates,
                                                          missedPublicationTimers,
                                                          subscriptionEndFutures,
                                                          subscriptionAttributeTypes,
                                                          cleanupScheduler);
        subscriptionId = "testSubscription";

        attributeName = "testAttribute";
        attributeSubscriptionCallback = new SubscriptionListener<Integer>() {
            @Override
            public void publicationMissed() {
                // TODO Auto-generated method stub
            }

            @Override
            public void receive(Integer value) {
                // TODO Auto-generated method stub

            }
        };
        long maxInterval_ms = 5000;
        long endDate_ms = System.currentTimeMillis() + 20000;
        long alertInterval_ms = 6000;
        long publicationTtl_ms = 1000;
        qos = new PeriodicSubscriptionQos(maxInterval_ms, endDate_ms, alertInterval_ms, publicationTtl_ms);
        missedPublicationTimer = new MissedPublicationTimer(endDate_ms,
                                                            alertInterval_ms,
                                                            attributeSubscriptionCallback,
                                                            subscriptionState);
    }

    @Test
    public void registerSubscription() {
        class IntegerReference extends TypeReference<Integer> {
        }

        subscriptionId = subscriptionManager.registerAttributeSubscription(attributeName,
                                                                           IntegerReference.class,
                                                                           attributeSubscriptionCallback,
                                                                           qos);

        Mockito.verify(attributeSubscriptionDirectory).put(Mockito.anyString(),
                                                           Mockito.eq(attributeSubscriptionCallback));
        Mockito.verify(subscriptionStates).put(Mockito.anyString(), Mockito.any(PubSubState.class));

        Mockito.verify(cleanupScheduler).schedule(Mockito.any(Runnable.class),
                                                  Mockito.eq(qos.getExpiryDate()),
                                                  Mockito.eq(TimeUnit.MILLISECONDS));
    }

    @Test
    public void unregisterSubscription() {

        Mockito.when(subscriptionStates.get(subscriptionId)).thenReturn(subscriptionState);
        Mockito.when(missedPublicationTimers.containsKey(subscriptionId)).thenReturn(true);
        Mockito.when(missedPublicationTimers.get(subscriptionId)).thenReturn(missedPublicationTimer);
        subscriptionManager.unregisterAttributeSubscription(subscriptionId);

        Mockito.verify(subscriptionStates).get(Mockito.eq(subscriptionId));
        Mockito.verify(subscriptionState).stop();
    }

    @Test
    public void touchSubscriptionState() {
        Mockito.when(subscriptionStates.containsKey(subscriptionId)).thenReturn(true);
        Mockito.when(subscriptionStates.get(subscriptionId)).thenReturn(subscriptionState);
        subscriptionManager.touchSubscriptionState(subscriptionId);

        Mockito.verify(subscriptionStates).containsKey(subscriptionId);
        Mockito.verify(subscriptionStates).get(subscriptionId);
        Mockito.verify(subscriptionState).updateTimeOfLastPublication();

    }
}
