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
#include "joynr/SubscriptionQos.h"
#include "joynr/DispatcherUtils.h"

namespace joynr {

const qint64& SubscriptionQos::DEFAULT_PUBLICATION_TTL() {
    static const qint64 defaultPublicationTtl = 10000;
    return defaultPublicationTtl;
}

const qint64& SubscriptionQos::MIN_PUBLICATION_TTL() {
    static const qint64 minPublicationTtl = 100;
    return minPublicationTtl;
}

const qint64& SubscriptionQos::MAX_PUBLICATION_TTL() {
    static const qint64 maxPublicationTtl = 2592000000UL;
    return maxPublicationTtl;
}

const qint64& SubscriptionQos::NO_EXPIRY_DATE() {
    static qint64 noExpiryDate = 0;
    return noExpiryDate;
}

SubscriptionQos::SubscriptionQos():
    expiryDate(-1),
    publicationTtl(DEFAULT_PUBLICATION_TTL())
{
    setValidity(1000);
}

SubscriptionQos::SubscriptionQos(const qint64& validity) :
    expiryDate(-1),
    publicationTtl(DEFAULT_PUBLICATION_TTL())
{
    setValidity(validity);
}

SubscriptionQos::SubscriptionQos(const SubscriptionQos& subscriptionQos) :
    QObject(),
    expiryDate(subscriptionQos.expiryDate),
    publicationTtl(subscriptionQos.publicationTtl)
{
}

SubscriptionQos::~SubscriptionQos()
{
}

qint64 SubscriptionQos::getPublicationTtl() const {
    return publicationTtl;
}

void SubscriptionQos::setPublicationTtl(const qint64& publicationTtl){
    this->publicationTtl = publicationTtl;
    if(this->publicationTtl > MAX_PUBLICATION_TTL()) {
        this->publicationTtl = MAX_PUBLICATION_TTL();
    }
    if(this->publicationTtl < MIN_PUBLICATION_TTL()) {
        this->publicationTtl = MIN_PUBLICATION_TTL();
    }
}

qint64 SubscriptionQos::getExpiryDate() const {
    return expiryDate;
}

void SubscriptionQos::setExpiryDate(const qint64& expiryDate){
    this->expiryDate = expiryDate;
    if (this->expiryDate < QDateTime::currentMSecsSinceEpoch()) {
        clearExpiryDate();
    }
}

void SubscriptionQos::clearExpiryDate() {
    this->expiryDate = NO_EXPIRY_DATE();
}

void SubscriptionQos::setValidity(const qint64 &validty) {
    setExpiryDate(QDateTime::currentMSecsSinceEpoch() + validty);
}

SubscriptionQos& SubscriptionQos::operator=(const SubscriptionQos& subscriptionQos) {
    expiryDate = subscriptionQos.getExpiryDate();
    publicationTtl = subscriptionQos.getPublicationTtl();
    return *this;
}

bool SubscriptionQos::operator==(const SubscriptionQos& subscriptionQos) const {
    return
        getExpiryDate() == subscriptionQos.getExpiryDate() &&
        publicationTtl == subscriptionQos.getPublicationTtl();
}

bool SubscriptionQos::equals(const QObject &other) const {
    int typeThis = QMetaType::type(this->metaObject()->className());
    int typeOther = QMetaType::type(other.metaObject()->className());
    auto newOther = dynamic_cast<const SubscriptionQos*>(&other);
    return typeThis == typeOther && *this == *newOther;
}
} // namespace joynr
