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
#include <QUuid>


#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils/TestQString.h"
#include "utils/QThreadSleep.h"


#include "joynr/IDispatcher.h"
#include "joynr/IMessaging.h"

#include "joynr/MessagingQos.h"
#include "joynr/JoynrMessage.h"
#include "joynr/JoynrMessageFactory.h"
#include "joynr/JoynrMessageSender.h"
#include "joynr/Request.h"
#include "joynr/Reply.h"
#include "joynr/SubscriptionPublication.h"
#include "joynr/PeriodicSubscriptionQos.h"
#include "tests/utils/MockObjects.h"

using ::testing::A;
using ::testing::_;
using ::testing::A;
using ::testing::Eq;
using ::testing::NotNull;
using ::testing::AllOf;
using ::testing::Property;
using namespace joynr;

class JoynrMessageSenderTest : public ::testing::Test {
public:
    JoynrMessageSenderTest() :
        messageFactory(),
        postFix(),
        senderID(),
        receiverID(),
        requestID(),
        qosSettings(),
        mockDispatcher(),
        mockMessagingStub(),
        callBack()
    {}


    void SetUp(){
        postFix = QString("_" + QUuid::createUuid().toString());
        senderID = QString("senderId" + postFix);
        receiverID = QString("receiverID" + postFix);
        requestID = QString("requestId" + postFix);
        qosSettings = MessagingQos(456000);
    }
    void TearDown(){

    }

protected:
    JoynrMessageFactory messageFactory;
    QString postFix;
    QString senderID;
    QString receiverID;
    QString requestID;
    MessagingQos qosSettings;
    MockDispatcher mockDispatcher;
    MockMessaging mockMessagingStub;
    QSharedPointer<IReplyCaller> callBack;

};

typedef JoynrMessageSenderTest JoynrMessageSenderDeathTest;


TEST_F(JoynrMessageSenderTest, sendRequest_normal){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());

    Request request;
    request.setMethodName("methodName");
    QList<QVariant> params;
    params.append(42);
    params.append("value");
    request.setParams(params);
    QList<QVariant> paramDatatypes;
    paramDatatypes.append("java.lang.Integer");
    paramDatatypes.append("java.lang.String");
    request.setParamDatatypes(paramDatatypes);

    JoynrMessage message = messageFactory.createRequest(
                senderID,
                receiverID,
                qosSettings,
                request
    );

    EXPECT_CALL( *(messagingStubQsp.data()), route(AllOf(Property(&JoynrMessage::getType, Eq(JoynrMessage::VALUE_MESSAGE_TYPE_REQUEST)),
                                                  Property(&JoynrMessage::getPayload, Eq(message.getPayload()))),
                                            qosSettings));

    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);
    joynrMessageSender.sendRequest(senderID, receiverID, qosSettings, request, callBack);
}

TEST_F(JoynrMessageSenderDeathTest, DISABLED_sendRequest_nullPayloadFails_death){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());
    EXPECT_CALL(*(messagingStubQsp.data()), route(_,_)).Times(0);

    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);

    Request jsonRequest;
    ASSERT_DEATH(joynrMessageSender.sendRequest(senderID, receiverID, qosSettings, jsonRequest, callBack), "Assertion.*");
}


TEST_F(JoynrMessageSenderTest, sendReply_normal){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());

    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);
    Reply reply;
    reply.setRequestReplyId(QUuid::createUuid().toString());
    reply.setResponse("response");

    JoynrMessage message = messageFactory.createReply(senderID,receiverID, qosSettings, reply);


    EXPECT_CALL(*(messagingStubQsp.data()), route(AllOf(Property(&JoynrMessage::getType, Eq(JoynrMessage::VALUE_MESSAGE_TYPE_REPLY)),
                                                  Property(&JoynrMessage::getPayload, Eq(message.getPayload()))),
                                            qosSettings));

    joynrMessageSender.sendReply(senderID, receiverID, qosSettings, reply);
}

TEST_F(JoynrMessageSenderTest, sendSubscriptionRequest_normal){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());

    qint64 period = 2000;
    qint64 validity = 100000;
    qint64 alert = 4000;
    auto qos = QSharedPointer<SubscriptionQos>(new PeriodicSubscriptionQos(validity, period, alert));

    SubscriptionRequest subscriptionRequest;
    subscriptionRequest.setSubscriptionId(QString("subscriptionId"));
    subscriptionRequest.setAttributeName(QString("attributeName"));
    subscriptionRequest.setQos(qos);

    JoynrMessage message = messageFactory.createSubscriptionRequest(senderID,receiverID, qosSettings, subscriptionRequest);


    EXPECT_CALL(*messagingStubQsp, route(AllOf(Property(&JoynrMessage::getType, Eq(JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REQUEST)),
                                                  Property(&JoynrMessage::getPayload, Eq(message.getPayload()))),
                                            qosSettings));

    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);

    joynrMessageSender.sendSubscriptionRequest(senderID, receiverID, qosSettings, subscriptionRequest);
}


//TODO implement sending a reply to a subscription request!
TEST_F(JoynrMessageSenderTest, DISABLED_sendSubscriptionReply_normal){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());
    QVariant payload = QVariant("subscriptionReply");
    EXPECT_CALL(*(messagingStubQsp.data()), route(AllOf(Property(&JoynrMessage::getType, Eq(JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REPLY)),
                                                  Property(&JoynrMessage::getPayload, Eq(payload))),
                                            qosSettings));



    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);

//    joynrMessageSender.sendSubscriptionReply(QUuid::createUuid().toString(), payload, senderID, receiverID, qosSettings);
}

TEST_F(JoynrMessageSenderTest, sendPublication_normal){

    MockDispatcher mockDispatcher;
    QSharedPointer<MockMessageRouter> messagingStubQsp(new MockMessageRouter());

    JoynrMessageSender joynrMessageSender(messagingStubQsp);
    joynrMessageSender.registerDispatcher(&mockDispatcher);
    SubscriptionPublication publication;
    publication.setSubscriptionId("ignoresubscriptionid");
    publication.setResponse("publication");
    JoynrMessage message = messageFactory.createSubscriptionPublication(senderID,receiverID, qosSettings, publication);

    EXPECT_CALL(*(messagingStubQsp.data()), route(AllOf(Property(&JoynrMessage::getType, Eq(JoynrMessage::VALUE_MESSAGE_TYPE_PUBLICATION)),
                                                      Property(&JoynrMessage::getPayload, Eq(message.getPayload()))),
                                                qosSettings));

    joynrMessageSender.sendSubscriptionPublication(senderID, receiverID, qosSettings, publication);
}
