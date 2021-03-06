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
#include "joynr/InProcessDispatcher.h"

#include "joynr/MessagingQos.h"
#include "joynr/JoynrMessage.h"

namespace joynr {

using namespace joynr_logging;

Logger* InProcessDispatcher::logger = Logging::getInstance()->getLogger("MSG", "InProcessDispatcher");


InProcessDispatcher::InProcessDispatcher()
    : requestCallerDirectory(QString("InProcessDispatcher-RequestCallerDirectory")),
      replyCallerDirectory(QString("InProcessDispatcher-ReplyCallerDirectory")),
      publicationManager(),
      subscriptionManager()

{

}

InProcessDispatcher::~InProcessDispatcher(){
    LOG_TRACE(logger, "Deleting InProcessDispatcher");
}

void InProcessDispatcher::addReplyCaller(const QString &requestReplyId, QSharedPointer<IReplyCaller> replyCaller, const MessagingQos& qosSettings){
    replyCallerDirectory.add(requestReplyId,
                             replyCaller,
                             qosSettings.getTtl());

}

void InProcessDispatcher::removeReplyCaller(const QString &requestReplyId){
    replyCallerDirectory.remove(requestReplyId);
}

void InProcessDispatcher::addRequestCaller(const QString &participantId, QSharedPointer<RequestCaller> requestCaller){
      requestCallerDirectory.add(participantId, requestCaller);
    // check if there are subscriptions waiting for this provider
    //publicationManager->restore(participantId,requestCaller, messageSender);
}

void InProcessDispatcher::removeRequestCaller(const QString &participantId){
    requestCallerDirectory.remove(participantId);
}

void InProcessDispatcher::receive(const JoynrMessage &message, const MessagingQos& qosSettings){
    Q_UNUSED(message);
    Q_UNUSED(qosSettings);
    LOG_FATAL(logger, "Not implemented");
    assert(false);

}

QSharedPointer<RequestCaller> InProcessDispatcher::lookupRequestCaller(const QString& participantId){
    return requestCallerDirectory.lookup(participantId);
}

bool InProcessDispatcher::containsRequestCaller(const QString &participantId){
    return requestCallerDirectory.contains(participantId);
}


void InProcessDispatcher::registerSubscriptionManager(SubscriptionManager* subscriptionManager) {
    this->subscriptionManager = subscriptionManager;
}

void InProcessDispatcher::registerPublicationManager(PublicationManager* publicationManager) {
    this->publicationManager = publicationManager;
}

} // namespace joynr
