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
#include "joynr/MessagingQos.h"
#include "QDateTime"
//#include <JoynrMessaging/messagingqos.h>

#include "joynr/DispatcherUtils.h"

namespace joynr {

MessagingQos::MessagingQos(const MessagingQos& other)
    : QObject(),
      ttl(other.ttl)
{
}

MessagingQos::MessagingQos(qint64 ttl)
    : ttl(ttl)
{
}

qint64 MessagingQos::getTtl() const {
    return ttl;
}

void MessagingQos::setTtl(const qint64 &ttl) {
    this->ttl = ttl;
}

bool MessagingQos::operator==(const MessagingQos& other) const{
    return this->getTtl() == other.getTtl();
}

MessagingQos& MessagingQos::operator=(const MessagingQos& other) {
    ttl = other.getTtl();
    return *this;
}

} // namespace joynr
