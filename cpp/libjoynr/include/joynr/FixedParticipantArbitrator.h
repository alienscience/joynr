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
#ifndef FIXEDPARTICIPANTARBITRATOR_H
#define FIXEDPARTICIPANTARBITRATOR_H
#include "joynr/PrivateCopyAssign.h"

#include "joynr/ProviderArbitrator.h"
#include <QSharedPointer>
#include <QString>

namespace joynr {

class FixedParticipantArbitrator : public ProviderArbitrator {
public:
    virtual ~FixedParticipantArbitrator() { }
    FixedParticipantArbitrator(const QString& domain,const QString& interfaceName, QSharedPointer<ICapabilities> capabilitiesStub,const DiscoveryQos &discoveryQos);

    /*
     * Attempt to arbitrate with a set participant id
     */
    void attemptArbitration();

private:
    DISALLOW_COPY_AND_ASSIGN(FixedParticipantArbitrator);
    QString participantId;
    qint64 reqCacheDataFreshness;
};


} // namespace joynr
#endif //FIXEDPARTICIPANTARBITRATOR_H
