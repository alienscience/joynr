package joynr;

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

import io.joynr.pubsub.HeartbeatSubscriptionInformation;
import io.joynr.pubsub.SubscriptionQos;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.annotation.JsonIgnore;

public class PeriodicSubscriptionQos extends SubscriptionQos implements HeartbeatSubscriptionInformation {
    private static final Logger logger = LoggerFactory.getLogger(PeriodicSubscriptionQos.class);

    private static final long MIN_PERIOD = 50L;
    private static final long MAX_PERIOD = 30L * 24L * 60L * 60L * 1000L; // 30 days

    private static final long MIN_ALERT_AFTER_INTERVAL = MIN_PERIOD;
    private static final long MAX_ALERT_AFTER_INTERVAL = 30L * 24L * 60L * 60L * 1000L; // 30 days
    private static final long DEFAULT_ALERT_AFTER_INTERVAL = 0L; // no alert
    private static final long NO_ALERT_AFTER_INTERVAL = 0L;

    private long period;
    private long alertAfterInterval;

    protected PeriodicSubscriptionQos() {
    }

    /**
     * @param period
     *            defines how long to wait before sending an update even if the value did not change or when onChange is
     *            false
     * @param expiryDate
     *            how long is the subscription valid
     * @param alertAfterInterval
     *            defines how long to wait for an update before publicationMissed is called
     * @param publicationTtl
     *            time to live for publication messages
     * 
     * @see #setPeriod(long)
     * @see #setExpiryDate(long)
     * @see #setAlertAfterInterval(long)
     * @see #setPublicationTtl(long)
     */
    public PeriodicSubscriptionQos(long period, long expiryDate, long alertAfterInterval, long publicationTtl) {
        super(expiryDate, publicationTtl);
        setPeriod(period);
        setAlertAfterInterval(alertAfterInterval);
    }

    public PeriodicSubscriptionQos(long period, long expiryDate, long publicationTtl) {
        this(period, expiryDate, DEFAULT_ALERT_AFTER_INTERVAL, publicationTtl);
    }

    /**
     * If no notification was received within the last alert interval, a missed publication notification will be raised.
     * 
     * @return alertAfterInterval_ms If more than alertAfterInterval_ms Milliseconds pass without receiving a message,
     *         the subscriptionManager will issue a publicationMissed. If set to 0 never alert.
     */
    public long getAlertAfterInterval() {
        return alertAfterInterval;
    }

    /**
     * If no notification was received within the last alert interval, a missed publication notification will be raised.
     * 
     * <p>
     * <ul>
     * <li>The absolute minimum setting is {@value #MIN_ALERT_AFTER_INTERVAL} milliseconds. <br>
     * Any value less than this minimum will be treated at the absolute minimum setting of
     * {@value #MIN_ALERT_AFTER_INTERVAL} milliseconds.
     * <li>The absolute maximum setting is {@value #MAX_ALERT_AFTER_INTERVAL} milliseconds. <br>
     * Any value bigger than this maximum will be treated at the absolute maximum setting of
     * {@value #MAX_ALERT_AFTER_INTERVAL} milliseconds.
     * </ul>
     * 
     * <p>
     * Use {@link #clearAlertAfterInterval()} to remove missed publication notifications.
     * 
     * @param alertAfterInterval_ms
     *            If more than alertInterval_ms pass without receiving a message, subscriptionManager will issue a
     *            publication missed.
     * 
     * @see #clearAlertAfterInterval()
     */
    public void setAlertAfterInterval(final long alertAfterInterval_ms) {
        if (alertAfterInterval_ms < MIN_ALERT_AFTER_INTERVAL) {
            this.alertAfterInterval = alertAfterInterval_ms;
            logger.warn("alertAfterInterval_ms < MIN_ALERT_AFTER_INTERVAL. Using MIN_ALERT_AFTER_INTERVAL: {}",
                        MIN_ALERT_AFTER_INTERVAL);
            return;
        }
        if (alertAfterInterval_ms > MAX_ALERT_AFTER_INTERVAL) {
            this.alertAfterInterval = alertAfterInterval_ms;
            logger.warn("alertAfterInterval_ms > MAX_ALERT_AFTER_INTERVAL. Using MAX_ALERT_AFTER_INTERVAL: {}",
                        MAX_ALERT_AFTER_INTERVAL);
            return;
        }
        this.alertAfterInterval = alertAfterInterval_ms;
    }

    /**
     * Clears the alert interval. No missed publication notifications will be raised.
     */
    public void clearAlertAfterInterval() {
        alertAfterInterval = NO_ALERT_AFTER_INTERVAL;
    }

    /**
     * @return period_ms The provider will periodically send notifications within the given interval in milliseconds
     */
    public long getPeriod() {
        return period;
    }

    /**
     * The provider will periodically send notifications within the given interval in milliseconds.
     * 
     * <p>
     * <ul>
     * <li>The absolute minimum setting is {@value #MIN_PERIOD} milliseconds. <br>
     * Any value less than this minimum will be treated at the absolute minimum setting of {@value #MIN_PERIOD}
     * milliseconds.
     * <li>The absolute maximum setting is {@value #MAX_PERIOD} milliseconds. <br>
     * Any value bigger than this maximum will be treated at the absolute maximum setting of {@value #MAX_PERIOD}
     * milliseconds.
     * </ul>
     * 
     * @param period_ms
     *            The publisher will send a notification at least every period_ms.
     * 
     */
    public void setPeriod(long period_ms) {
        if (period_ms < MIN_PERIOD) {
            this.period = MIN_PERIOD;
            logger.warn("alertAfterInterval_ms < MIN_PERIOD. Using MIN_PERIOD: {}", MIN_PERIOD);

            return;
        }
        if (period_ms > MAX_PERIOD) {
            this.period = MAX_PERIOD;
            logger.warn("alertAfterInterval_ms > MAX_PERIOD. Using MAX_PERIOD: {}", MAX_PERIOD);
            return;
        }
        this.period = period_ms;
    }

    @Override
    @JsonIgnore
    public long getHeartbeat() {
        return period;
    }

}
