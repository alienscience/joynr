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
#ifndef JOYNRMESSAGINGSTUB_H
#define JOYNRMESSAGINGSTUB_H
#include "joynr/PrivateCopyAssign.h"
#include "joynr/IMessaging.h"

#include <QString>

namespace joynr {

class ICommunicationManager;
class JoynrMessage;
/**
  * Is used by the ClusterController to contact another (remote) ClusterController
  */
class JoynrMessagingStub : public IMessaging {
public:
    explicit JoynrMessagingStub(ICommunicationManager& communicationManager, QString destChannelId);
    virtual ~JoynrMessagingStub();
    void transmit(JoynrMessage& message, const MessagingQos& qos);
private:
    DISALLOW_COPY_AND_ASSIGN(JoynrMessagingStub);
    ICommunicationManager& communicationManager;
    QString destinationChannelId;
};

} // namespace joynr
#endif //JOYNRMESSAGINGSTUB_H
