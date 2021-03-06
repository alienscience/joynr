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
#ifndef RECEIVEDMESSAGERUNNABLE_H
#define RECEIVEDMESSAGERUNNABLE_H
#include "joynr/PrivateCopyAssign.h"
#include "joynr/joynrlogging.h"
#include "joynr/ObjectWithDecayTime.h"
#include "joynr/JoynrMessage.h"
#include "joynr/MessagingQos.h"

#include <QRunnable>

namespace joynr {

class Dispatcher;

/**
  * ReceivedMessageRunnable are used handle an incoming message via a ThreadPool.
  *
  */

class ReceivedMessageRunnable : public QRunnable, public ObjectWithDecayTime {
public:
    ReceivedMessageRunnable(
            const QDateTime& decayTime,
            const JoynrMessage& message,
            const MessagingQos& qos,
            Dispatcher& dispatcher);

    void run();
private:
    DISALLOW_COPY_AND_ASSIGN(ReceivedMessageRunnable);
    JoynrMessage message;
    MessagingQos qos;
    Dispatcher& dispatcher;
    static joynr_logging::Logger* logger;

};


} // namespace joynr
#endif // RECEIVEDMESSAGERUNNABLE_H
