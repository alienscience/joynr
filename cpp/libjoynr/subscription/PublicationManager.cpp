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
#include "joynr/PublicationManager.h"
#include "joynr/RequestCaller.h"
#include "joynr/DispatcherUtils.h"
#include "joynr/JsonSerializer.h"
#include "joynr/IRequestInterpreter.h"
#include "joynr/InterfaceRegistrar.h"
#include "joynr/SubscriptionPublication.h"
#include "joynr/DelayedScheduler.h"
#include "joynr/MessagingQos.h"
#include "joynr/IPublicationSender.h"
#include "joynr/SubscriptionRequest.h"
#include "joynr/Util.h"
#include "libjoynr/subscription/SubscriptionRequestInformation.h"
#include "libjoynr/subscription/SubscriptionAttributeListener.h"

#include "joynr/SubscriptionUtil.h"

#include <QFile>
#include <cassert>

namespace joynr {

//------ HelperClasses ---------------------------------------------------------

class PublicationManager::Publication {
public:
    Publication(IPublicationSender* publicationSender,
                QSharedPointer<RequestCaller> requestCaller);
    ~Publication();

    qint64 timeOfLastPublication;
    IPublicationSender* sender;
    QSharedPointer<RequestCaller> requestCaller;
    SubscriptionAttributeListener* attributeListener;
private:
    DISALLOW_COPY_AND_ASSIGN(Publication);
};

class PublicationManager::PublisherRunnable : public QRunnable {
public:
    virtual ~PublisherRunnable();
    PublisherRunnable(PublicationManager& publicationManager,
                      const QString& subscriptionId);

    // Calls PublicationManager::pollSubscription()
    void run();
private:
    DISALLOW_COPY_AND_ASSIGN(PublisherRunnable);
    PublicationManager& publicationManager;
    QString subscriptionId;
};

class PublicationManager::PublicationEndRunnable : public QRunnable {
public:
    virtual ~PublicationEndRunnable();
    PublicationEndRunnable(PublicationManager& publicationManager,
                           const QString& subscriptionId);

    // Calls PublicationManager::removePublication()
    void run();
private:
    DISALLOW_COPY_AND_ASSIGN(PublicationEndRunnable);
    PublicationManager& publicationManager;
    QString subscriptionId;
};


//------ PublicationManager ----------------------------------------------------

using namespace joynr_logging;
Logger* PublicationManager::logger = Logging::getInstance()->getLogger("MSG", "PublicationManager");

PublicationManager::~PublicationManager() {
    LOG_DEBUG(logger, "Destructor, saving subscriptionsMap...");
    saveSubscriptionRequestsMap();

    // saveSubscriptionRequestsMap will not store to file, as soon as shuttingDown is true, so we call it first
    //then set shuttingDown to true
    {
        QMutexLocker locker(&shutDownMutex);
        shuttingDown = true;
    }

    LOG_DEBUG(logger, "Destructor, waiting for thread pool ...");
    publishingThreadPool.waitForDone();

    LOG_DEBUG(logger, "Destructor, deleting scheduler ...");
    delete delayedScheduler;

    // Remove all publications
    LOG_DEBUG(logger, "Destructor: removing publications");
    foreach(SubscriptionRequestInformation* request, subscriptionId2SubscriptionRequest) {
        removePublication(request->getSubscriptionId());
    }
}

PublicationManager::PublicationManager(int maxThreads)
    : publications(),
      subscriptionId2SubscriptionRequest(),
      subscriptionLock(),
      publishingThreadPool(),
      delayedScheduler(NULL),
      shutDownMutex(),
      shuttingDown(false),
      subscriptionRequestStorageFileName("SubscriptionRequests.persist"),
      queuedSubscriptionRequests(),
      queuedSubscriptionRequestsMutex(),
      currentScheduledPublications(),
      currentScheduledPublicationsMutex()
{

    publishingThreadPool.setMaxThreadCount(maxThreads);
    delayedScheduler = new ThreadPoolDelayedScheduler(publishingThreadPool, QString("PublicationManager-PublishingThreadPool"));
    qRegisterMetaType<SubscriptionRequest>("SubscriptionRequest");
    qRegisterMetaType<QSharedPointer<SubscriptionRequest> >("QSharedPointer<SubscriptionRequest>");
    loadSavedSubscriptionRequestsMap();
}

void PublicationManager::add(
        const QString& proxyParticipantId,
        const QString& providerParticipantId,
        QSharedPointer<RequestCaller> requestCaller,
        SubscriptionRequest* subscriptionRequest,
        IPublicationSender* publicationSender
) {
    assert(!requestCaller.isNull());
    SubscriptionRequestInformation* requestInfo = new SubscriptionRequestInformation(
                proxyParticipantId,
                providerParticipantId,
                *subscriptionRequest
    );
    delete subscriptionRequest;

    if (publicationExists(requestInfo->getSubscriptionId())){
        LOG_WARN(logger, "Publication with id: " + requestInfo->getSubscriptionId()
             + " already exists.");
        delete requestInfo;
        return;
    }

    {
        QWriteLocker locker(&subscriptionLock);
        QString subscriptionId = requestInfo->getSubscriptionId();
        subscriptionId2SubscriptionRequest.insert(subscriptionId, requestInfo);

        // Make note of the publication
        Publication *publication = new Publication(publicationSender, requestCaller);
        publications.insert(subscriptionId, publication);
        LOG_DEBUG(logger, QString("added subscription: %1").arg(requestInfo->toQString()));

        // Add an onChange publication if needed
        addOnChangePublication(subscriptionId, requestInfo, publication);

        // Schedule a runnable to remove the publication when it finishes
        QSharedPointer<SubscriptionQos> qos = requestInfo->getQos();
        qint64 publicationEndDelay = qos->getExpiryDate() - QDateTime::currentMSecsSinceEpoch();

        // check for a valid publication end date
        if(publicationEndDelay > 0) {
            delayedScheduler->schedule(new PublicationEndRunnable(*this, subscriptionId), publicationEndDelay);
            LOG_DEBUG(logger, QString("publication will end in %1 ms").arg(publicationEndDelay));
            {
                QMutexLocker currentScheduledLocker(&currentScheduledPublicationsMutex);
                currentScheduledPublications.append(subscriptionId);
            }
            //sent at least once the current value
            delayedScheduler->schedule(new PublisherRunnable(*this, subscriptionId), -1);
        } else {
            LOG_WARN(logger, QString("publication end is in the past"));
        }

    }
    saveSubscriptionRequestsMap();
}

void PublicationManager::addOnChangePublication(const QString& subscriptionId,
                                                SubscriptionRequestInformation* request,
                                                Publication* publication)
{
    if(SubscriptionUtil::isOnChangeSubscription(request->getQos().data())) {
        LOG_TRACE(logger, QString("adding onChange subscription: %1").arg(subscriptionId));

        // Create an attribute listener to listen for onChange events
        SubscriptionAttributeListener* attributeListener = new SubscriptionAttributeListener(subscriptionId, *this);

        // Register the attribute listener
        QSharedPointer<RequestCaller> requestCaller = publication->requestCaller;
        requestCaller->registerAttributeListener(request->getAttributeName(), attributeListener);

        // Make note of the attribute listener so that it can be unregistered
        publication->attributeListener = attributeListener;
    }
}

void PublicationManager::add(
        const QString& proxyParticipantId,
        const QString& providerParticipantId,
        SubscriptionRequest *subscriptionRequest
) {
    LOG_DEBUG(logger, "Added subscription for non existing provider (adding subscriptionRequest to queue).");
    SubscriptionRequestInformation* requestInfo = new SubscriptionRequestInformation(
                proxyParticipantId,
                providerParticipantId,
                *subscriptionRequest
    );
    delete subscriptionRequest;
    {
        QMutexLocker locker(&queuedSubscriptionRequestsMutex);
        queuedSubscriptionRequests.insert(
                    requestInfo->getProviderId(),
                    requestInfo);
    }
    {
        QWriteLocker locker(&subscriptionLock);
        subscriptionId2SubscriptionRequest.insert(
                requestInfo->getSubscriptionId(),
                requestInfo);
    }
    saveSubscriptionRequestsMap();
}


void PublicationManager::removeAllSubscriptions(const QString& providerId) {
    LOG_DEBUG(logger, QString("Removing all subscriptions for provider id= %1")
                            .arg(providerId));

    // Build a list of subscriptionIds to remove
    QString subscriptionId;
    QList<QString> toRemove;
    {
        QReadLocker locker(&subscriptionLock);

        foreach (SubscriptionRequestInformation* requestInfo, subscriptionId2SubscriptionRequest) {
            subscriptionId = requestInfo->getSubscriptionId();

            if(requestInfo->getProviderId() == providerId) {
                toRemove.append(subscriptionId);
            }
        }
    }

    // Remove each publication
    foreach (subscriptionId, toRemove) {
        LOG_DEBUG(logger, QString("Removing subscription providerId= %1, subscriptionId =%2")
                                .arg(providerId)
                                .arg(subscriptionId));
        removePublication(subscriptionId);
    }
}


void PublicationManager::stopPublication(const QString& subscriptionId){
    LOG_DEBUG(logger, QString("stopPublication: %1").arg(subscriptionId));
    removePublication(subscriptionId);
}

bool PublicationManager::publicationExists(const QString& subscriptionId) const {
    return publications.contains(subscriptionId);
}

void PublicationManager::restore(const QString& providerId,
                                 QSharedPointer<RequestCaller> requestCaller,
                                 IPublicationSender* publicationSender)
{
    LOG_DEBUG(logger, "restore: entering ...");

    QList<SubscriptionRequestInformation*> subscriptions = queuedSubscriptionRequests.values(providerId);

    foreach (SubscriptionRequestInformation* requestInfo, subscriptions) {
        if (requestInfo->getQos()->getExpiryDate() > QDateTime::currentMSecsSinceEpoch()){
            LOG_DEBUG(logger,
                      QString("Restoring subscription for provider: %1 %2")
                        .arg(providerId)
                        .arg(requestInfo->toQString()));
            add(requestInfo->getProxyId(), requestInfo->getProviderId(), requestCaller, requestInfo, publicationSender);
        }
    }
}

// This function assumes that no lock is held
void PublicationManager::saveSubscriptionRequestsMap() {
    LOG_DEBUG(logger, "Saving active subscriptionRequests to file.");

    if (isShuttingDown()){
        LOG_DEBUG(logger, "Abort saving, because we are already shutting down.");
        return;
    }

    QFile file(subscriptionRequestStorageFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(logger, QString("Could not open subscription request storage file: %1")
                                 .arg(file.errorString()));
        return;
    }

    // Write the subscription information as a json list
    file.resize(0);

    QDateTime now = QDateTime::currentDateTime();

    QList<QVariant> subscriptionList;
    {
        QReadLocker locker(&subscriptionLock);

        foreach (SubscriptionRequestInformation* requestInfo, subscriptionId2SubscriptionRequest) {
            if(requestInfo->getQos()->getExpiryDate() > now.toMSecsSinceEpoch()) {
                subscriptionList.append( QVariant::fromValue(*requestInfo) );
            }
        }
    }
    QString json = JsonSerializer::serialize(subscriptionList);
    file.write( json.toUtf8().constData() );
}

void PublicationManager::loadSavedSubscriptionRequestsMap(){
    LOG_DEBUG(logger, "Loading stored Subscriptionrequests.");

    QFile file(subscriptionRequestStorageFileName);
    if (!file.open(QIODevice::ReadOnly)){
        LOG_ERROR(logger, QString("Unable to read file: %1").arg(file.errorString()));
        return;
    }

    // Read the Json into memory
    QByteArray jsonBytes = file.readAll();

    // Deserialize the JSON into a list of subscription requests
    QList<SubscriptionRequestInformation*> subscriptionList =
            JsonSerializer::deserializeList<SubscriptionRequestInformation>(jsonBytes);

    // Loop through the saved subscriptions
    QDateTime now = QDateTime::currentDateTime();
    QMutexLocker locker(&queuedSubscriptionRequestsMutex);

    while(!subscriptionList.isEmpty()) {
        SubscriptionRequestInformation* requestInfo = subscriptionList.takeFirst();

        // Add the subscription if it is still valid
        if (requestInfo->getQos()->getExpiryDate() > now.toMSecsSinceEpoch()){
            QString providerId = requestInfo->getProviderId();
            queuedSubscriptionRequests.insertMulti(providerId, requestInfo);
            LOG_DEBUG(logger, QString("Queuing subscription Request: %1 : %2")
                      .arg(providerId)
                      .arg(requestInfo->toQString()));
        } else {
            // delete the subscription request
            delete requestInfo;
        }
    }
}


void PublicationManager::removePublication(const QString& subscriptionId) {
    LOG_DEBUG(logger, QString("removePublication: %1").arg(subscriptionId));

    {
        QWriteLocker locker(&subscriptionLock);

        if (!publicationExists(subscriptionId)) {
            LOG_DEBUG(logger, QString("publication %1 does not exist - will not remove").arg(subscriptionId));
            return;
        }

        Publication* publication = publications.take(subscriptionId);
        SubscriptionRequestInformation* request = subscriptionId2SubscriptionRequest.value(subscriptionId);

        // Delete the onChange publication if needed
        removeOnChangePublication(subscriptionId, request, publication);

        // Remove the publication
        subscriptionId2SubscriptionRequest.remove(subscriptionId);
        delete request;
        delete publication;
    }
    saveSubscriptionRequestsMap();
}

// This function assumes that a write lock is already held
void PublicationManager::removeOnChangePublication(const QString& subscriptionId,
                                                   SubscriptionRequestInformation* request,
                                                   Publication* publication)
{
    if (SubscriptionUtil::isOnChangeSubscription(request->getQos().data())) {
        LOG_DEBUG(logger, QString("Removing onChange publication for id = %1").arg(subscriptionId));

        // Unregister and delete the attribute listener
        QSharedPointer<RequestCaller> requestCaller = publication->requestCaller;
        requestCaller->unregisterAttributeListener(request->getAttributeName(), publication->attributeListener);
        publication->attributeListener = NULL;
    }
}

bool PublicationManager::isShuttingDown() {
    QMutexLocker locker(&shutDownMutex);
    return shuttingDown;
}

qint64 PublicationManager::getPublicationTtl(SubscriptionRequestInformation* subscriptionRequest) const {
    return subscriptionRequest->getQos()->getPublicationTtl();
}

// This function assumes that a lock is held
void PublicationManager::sendPublication(const QString& subscriptionId,
                                         SubscriptionRequestInformation* subscriptionRequest,
                                         const QVariant& value)
{
    LOG_DEBUG(logger, "sending subscriptionreply");
    MessagingQos mQos;

    // Set the TTL
    qint64 publicationTtl = getPublicationTtl(subscriptionRequest);
    mQos.setTtl(publicationTtl);

    // Get publication information
    Publication* publication = publications.value(subscriptionId);
    IPublicationSender* publicationSender = publication->sender;

    SubscriptionPublication subscriptionPublication;
    subscriptionPublication.setSubscriptionId(subscriptionId);
    subscriptionPublication.setResponse(value);
    publicationSender->sendSubscriptionPublication(
                subscriptionRequest->getProviderId(),
                subscriptionRequest->getProxyId(),
                mQos,
                subscriptionPublication
    );

    // Make note of when this publication was sent
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    publication->timeOfLastPublication = now;

    {
        QMutexLocker currentScheduledLocker(&currentScheduledPublicationsMutex);
        currentScheduledPublications.removeAll(subscriptionId);
    }

    LOG_TRACE(logger, QString("sent subscriptionreply @ %1").arg(now));
}

void PublicationManager::pollSubscription(const QString& subscriptionId)
{
    LOG_TRACE(logger, QString("pollSubscription %1").arg(subscriptionId));

    QWriteLocker locker(&subscriptionLock);

    // Check that the subscription has not been removed and that we are not shutting down
    if (isShuttingDown() || !publicationExists(subscriptionId)) {
        return;
    }

    // Get the subscription details
    Publication* publication = publications.value(subscriptionId);
    SubscriptionRequestInformation* subscriptionRequest = subscriptionId2SubscriptionRequest.value(subscriptionId);

    // See if the publication is needed
    QSharedPointer<SubscriptionQos> qos = subscriptionRequest->getQos();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 publicationInterval = SubscriptionUtil::getPeriodicPublicationInterval(qos.data());

    // check if the subscription qos needs a periodic publication
    if(publicationInterval > 0) {
        qint64 timeSinceLast = now - publication->timeOfLastPublication;
        // publish only if not published in the current interval
        if(timeSinceLast < publicationInterval) {
            LOG_DEBUG(logger,  QString("no publication necessary. publicationInterval: %1, timeSinceLast %2")
                                .arg(publicationInterval)
                                .arg(timeSinceLast));

            qint64 delayUntilNextPublication = publicationInterval - timeSinceLast;
            assert(delayUntilNextPublication >=0);
            delayedScheduler->schedule(new PublisherRunnable(*this, subscriptionId),
                                        delayUntilNextPublication);
            return;
        }
    }

    // Get the value of the attribute
    QString attributeGetter = Util::attributeGetterFromName(subscriptionRequest->getAttributeName());
    QSharedPointer<RequestCaller> requestCaller = publication->requestCaller;
    QSharedPointer<IRequestInterpreter> requestInterpreter =
            InterfaceRegistrar::instance().getRequestInterpreter(requestCaller->getInterfaceName());

    LOG_DEBUG(logger, QString("run: executing requestInterpreter= %1").arg(attributeGetter));
    QVariant response = requestInterpreter->execute(requestCaller,
                                                    attributeGetter,
                                                    QList<QVariant>(),
                                                    QList<QVariant>());

    // Publish
    sendPublication(subscriptionId, subscriptionRequest, response);

    // Reschedule the next poll
    if (publicationInterval > 0 && qos->getExpiryDate() - now > publicationInterval ) {
        LOG_DEBUG(logger, QString("rescheduling runnable with delay: %1").arg(publicationInterval));
        delayedScheduler->schedule(new PublisherRunnable(*this, subscriptionId),
                                   publicationInterval);
    }
}


void PublicationManager::attributeValueChanged(const QString& subscriptionId, const QVariant& value)
{
    LOG_DEBUG(logger, QString("attributeValueChanged for onChange subscription %1").arg(subscriptionId));

    QReadLocker locker(&subscriptionLock);

    // See if the subscription is still valid
    if (!publicationExists(subscriptionId)) {
        LOG_ERROR(logger, QString("attributeValueChanged called for non-existing subscription %1").arg(subscriptionId));
        return;
    }

    // See if a publication is already scheduled
    {
        QMutexLocker currentScheduledLocker(&currentScheduledPublicationsMutex);
        if(currentScheduledPublications.contains(subscriptionId)) {
            LOG_DEBUG(logger, QString("publication runnable already scheduled"));
            return;
        }
    }

    // Check the last publication time against the min interval
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    SubscriptionRequestInformation* subscriptionRequest = subscriptionId2SubscriptionRequest.value(subscriptionId);
    Publication* publication = publications.value(subscriptionId);
    QSharedPointer<SubscriptionQos> qos = subscriptionRequest->getQos();
    qint64 minInterval = SubscriptionUtil::getMinInterval(qos.data());
    qint64 timeSinceLast = now - publication->timeOfLastPublication;

    if (minInterval > 0 && timeSinceLast < minInterval) {
        LOG_DEBUG(logger, QString("attributeValueChanged ignored on subscription %1, %2 < %3")
                            .arg(subscriptionId)
                            .arg(timeSinceLast)
                            .arg(minInterval));

        QMutexLocker currentScheduledLocker(&currentScheduledPublicationsMutex);

        // Schedule a publication so that the change is not forgotten
        if(!currentScheduledPublications.contains(subscriptionId)) {
            qint64 nextPublication = minInterval - timeSinceLast;
            LOG_DEBUG(logger, QString("rescheduling runnable with delay: %1").arg(nextPublication));
            currentScheduledPublications.append(subscriptionId);
            delayedScheduler->schedule(new PublisherRunnable(*this, subscriptionId), nextPublication);
        }

        return;
    }

    // Send the publication
    sendPublication(subscriptionId, subscriptionRequest, value);
}

//------ PublicationManager::Publication ---------------------------------------

PublicationManager::Publication::~Publication()
{
    // This class is not responsible for deleting the PublicationSender or AttributeListener
}


PublicationManager::Publication::Publication(IPublicationSender* publicationSender,
                                             QSharedPointer<RequestCaller> requestCaller) :
    timeOfLastPublication(0),
    sender(publicationSender),
    requestCaller(requestCaller),
    attributeListener(NULL)
{
}


//------ PublicationManager::PublisherRunnable ---------------------------------

PublicationManager::PublisherRunnable::~PublisherRunnable()
{
}


PublicationManager::PublisherRunnable::PublisherRunnable(PublicationManager& publicationManager,
                                                         const QString& subscriptionId) :
    publicationManager(publicationManager),
    subscriptionId(subscriptionId)
{
    setAutoDelete(true);
}


void PublicationManager::PublisherRunnable::run()
{
    publicationManager.pollSubscription(subscriptionId);
}

//------ PublicationManager::PublicationEndRunnable ----------------------------

PublicationManager::PublicationEndRunnable::~PublicationEndRunnable()
{
}


PublicationManager::PublicationEndRunnable::PublicationEndRunnable(
        PublicationManager& publicationManager,
        const QString& subscriptionId) :
    publicationManager(publicationManager),
    subscriptionId(subscriptionId)
{
    setAutoDelete(true);
}


void PublicationManager::PublicationEndRunnable::run()
{
    publicationManager.removePublication(subscriptionId);
}



} // namespace joynr
