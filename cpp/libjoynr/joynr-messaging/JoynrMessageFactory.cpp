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
#include "joynr/JoynrMessageFactory.h"
#include "joynr/DispatcherUtils.h"
#include "joynr/SubscriptionRequest.h"
#include "joynr/JsonSerializer.h"
#include "joynr/Request.h"
#include "joynr/Reply.h"
#include "joynr/SubscriptionPublication.h"
#include "joynr/SubscriptionReply.h"
#include "joynr/SubscriptionStop.h"
#include "joynr/Util.h"

namespace joynr {

JoynrMessageFactory::JoynrMessageFactory() :
    logger(joynr_logging::Logging::getInstance()->getLogger(QString("LIB"), QString("JoynrMessageFactory")))
{
    qRegisterMetaType<Reply>();
    qRegisterMetaType<Request>();
    qRegisterMetaType<SubscriptionRequest>();
    qRegisterMetaType<SubscriptionReply>();
    qRegisterMetaType<SubscriptionStop>();
    qRegisterMetaType<SubscriptionPublication>();
    qRegisterMetaType<JoynrMessage>();
}

JoynrMessage JoynrMessageFactory::createRequest(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const Request& payload
) {
    // create message and set type
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_REQUEST);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createReply(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const Reply& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_REPLY);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createOneWay(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const Reply& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_ONE_WAY);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createSubscriptionPublication(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const SubscriptionPublication& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_PUBLICATION);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createSubscriptionRequest(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const SubscriptionRequest& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REQUEST);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createSubscriptionReply(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const SubscriptionReply& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_REPLY);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

JoynrMessage JoynrMessageFactory::createSubscriptionStop(
        const QString& senderId,
        const QString& receiverId,
        const MessagingQos& qos,
        const SubscriptionStop& payload
) {
    JoynrMessage msg;
    msg.setType(JoynrMessage::VALUE_MESSAGE_TYPE_SUBSCRIPTION_STOP);
    initMsg(msg, senderId, receiverId, qos.getTtl(), payload);
    return msg;
}

void JoynrMessageFactory::initMsg(
        JoynrMessage& msg,
        const QString& senderParticipantId,
        const QString& receiverParticipantId,
        const qint64 ttl,
        const QObject& payload
) {
    msg.setHeaderFrom(senderParticipantId);
    msg.setHeaderTo(receiverParticipantId);

    // calculate expiry date
    QDateTime expiryDate = QDateTime::currentDateTime().addMSecs(ttl);
    msg.setHeaderExpiryDate(expiryDate);

    // add content type and class
    msg.setHeaderContentType(JoynrMessage::VALUE_CONTENT_TYPE_APPLICATION_JSON);

    // set payload
    msg.setPayload(JsonSerializer::serialize(payload));
}

} // namespace joynr
