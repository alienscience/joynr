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
#ifndef SUBSCRIPTIONREQUESTINFORMATION_H
#define SUBSCRIPTIONREQUESTINFORMATION_H


#include "joynr/JoynrExport.h"
#include "joynr/joynrlogging.h"
#include "joynr/SubscriptionRequest.h"

#include <QString>
#include <QSharedPointer>

namespace joynr {

class JOYNR_EXPORT SubscriptionRequestInformation : public SubscriptionRequest {
    Q_OBJECT

    Q_PROPERTY(QString proxyId READ getProxyId WRITE setProxyId)
    Q_PROPERTY(QString providerId READ getProviderId WRITE setProviderId)

public:
    SubscriptionRequestInformation();
    SubscriptionRequestInformation(
            const QString& proxyParticipantId,
            const QString& providerParticipantId,
            const SubscriptionRequest& subscriptionRequest
    );

    SubscriptionRequestInformation(const SubscriptionRequestInformation& subscriptionRequestInformation);
    virtual ~SubscriptionRequestInformation(){}

    SubscriptionRequestInformation& operator=(const SubscriptionRequestInformation& subscriptionRequestInformation);
    bool operator==(const SubscriptionRequestInformation& subscriptionRequestInformation) const;

    QString getProxyId() const;
    void setProxyId(const QString& id);

    QString getProviderId() const;
    void setProviderId(const QString& id);

    QString toQString() const;

private:
    QString proxyId;
    QString providerId;

    static joynr_logging::Logger* logger;
};

} // namespace joynr

Q_DECLARE_METATYPE(joynr::SubscriptionRequestInformation)
Q_DECLARE_METATYPE(QSharedPointer<joynr::SubscriptionRequestInformation>)

#endif // SUBSCRIPTIONREQUESTINFORMATION_H
