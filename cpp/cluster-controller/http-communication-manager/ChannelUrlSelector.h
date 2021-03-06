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
#ifndef CHANNELDIRECTORYURLCACHE_H_
#define CHANNELDIRECTORYURLCACHE_H_
#include "joynr/PrivateCopyAssign.h"

#include "joynr/JoynrClusterControllerExport.h"
#include "joynr/joynrlogging.h"
#include "cluster-controller/http-communication-manager/IChannelUrlSelector.h"
#include "joynr/ILocalChannelUrlDirectory.h"
#include "joynr/types/ChannelUrlInformation.h"
#include "joynr/BounceProxyUrl.h"

// Forward declare test classes
class ChannelUrlSelectorTest_punishTest_Test;
class ChannelUrlSelectorTest_updateTest_Test;
class ChannelUrlSelectorTest_initFittnessTest_Test;

namespace joynr {

class ChannelUrlSelectorEntry;


/**
 * @brief ChannelUrlSelector
 * Used by the MessageSender to obtain the 'best' Url available for a channelId. The 'best' is
 * determined using feedback from former trials. The available Urls for a channelId are ranked according to
 * their position, the first Url is ranked highest. Every Url is assigned a 'fitness' value.
 * This fitness is initialized to the rank of the Url. It cannot be higher than the rank of the
 * corrsponding Url. If a connection using an Url fails, its fitness value is reduced by 'punishMent' factor.
 * The 'best' Url is the Url with the highest fitness value. After 'timeForOneRecouperation'
 * has passed, the fitness value of all Urls are increased.
 *
 */
class JOYNRCLUSTERCONTROLLER_EXPORT ChannelUrlSelector : public IChannelUrlSelector{

public:
     static const qint64& TIME_FOR_ONE_RECOUPERATION();
     static const double& PUNISHMENT_FACTOR();
    /**
     * @brief Initialize
     *
     * @param bounceProxyUrl
     * @param timeForOneRecouperation
     * @param punishmentFactor
     */
    explicit ChannelUrlSelector(const BounceProxyUrl& bounceProxyUrl,
                                      qint64 timeForOneRecouperation,
                                      double punishmentFactor);

    virtual ~ChannelUrlSelector();

    /**
    * @brief Uses the ChannelUrlDirectoryProxy to query the remote ChannelUrlDirectory
    *
    * @param channelUrlDirectoryProxy
    */
    virtual void init(
             QSharedPointer<ILocalChannelUrlDirectory> channelUrlDirectory,
             const MessagingSettings& settings);

    /**
    * @brief Get the "best" URL for this channel. Feedback is used to figure out which
    * URL is currently best depending on recent availability and initial ordering (eg direct before
    * bounceproxy URL.
    *
    * @param channelId
    * @param status
    * @param timeout
    * @return QString
    */
    virtual QString obtainUrl(
            const QString& channelId,
            RequestStatus& status,
            const qint64& timeout_ms );
    /**
    * @brief Provide feedback on performance of URL: was the connection successful or not?
    *
    * @param success
    * @param channelId
    * @param url
    */
    virtual void feedback(bool success,
            const QString& channelId,
            QString url);


private:
    DISALLOW_COPY_AND_ASSIGN(ChannelUrlSelector);
    QString constructDefaultUrl(
            const QString& channelId);
    QString constructUrl(const QString& baseUrl);
    QSharedPointer<ILocalChannelUrlDirectory> channelUrlDirectory;
    const BounceProxyUrl& bounceProxyUrl;
    QMap<QString, ChannelUrlSelectorEntry*> entries;
    qint64 timeForOneRecouperation;
    double punishmentFactor;
    QString channelUrlDirectoryUrl;
    bool useDefaultUrl;
    static joynr_logging::Logger* logger;
};

/**
 * @brief ChannelUrlSelectorEntry
 *
 * This is a "private Class" of ChannelUrlSelector. In order to use it with googleTest it has been moved out of ChannelUrlSelector
 * Class, but stays within the same file, as noone else should use ChannelUrlSelector.
 *
 */
class JOYNRCLUSTERCONTROLLER_EXPORT ChannelUrlSelectorEntry  {
public:
    ChannelUrlSelectorEntry(const types::ChannelUrlInformation& urlInformation,
                                  double punishmentFactor,
                                  qint64 timeForOneRecouperation);
    ~ChannelUrlSelectorEntry();
    /**
     * @brief Returns the Url with the higest fitness value.
     *
     * @return QString
     */
    QString best();
    /**
     * @brief Reduces the fitness value of Url url.
     *
     * @param url
     */
    void punish(const QString& url);
    /**
     * @brief Initializes the fitness values, ranks the Urls according to their position
     * (first Url has highest rank).
     *
     */
    void initFitness();
    /**
     * @brief Checks whether time for one recouperation has passed and increases fitness values if so.
     *
     */
    void updateFitness();
    /**
     * @brief Returns the current fitness values.
     *
     */
    QList<double> getFitness();
private:
    DISALLOW_COPY_AND_ASSIGN(ChannelUrlSelectorEntry);

    friend class ::ChannelUrlSelectorTest_punishTest_Test;
    friend class ::ChannelUrlSelectorTest_updateTest_Test;
    friend class ::ChannelUrlSelectorTest_initFittnessTest_Test;

    qint64 lastUpdate;
    QList<double> fitness;
    types::ChannelUrlInformation urlInformation;
    double punishmentFactor;
    qint64 timeForOneRecouperation;
    static joynr_logging::Logger* logger;

};



} // namespace joynr
#endif //CHANNELDIRECTORYURLCACHE_H_
