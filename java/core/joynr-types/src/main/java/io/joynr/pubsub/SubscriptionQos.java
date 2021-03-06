package io.joynr.pubsub;

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

import io.joynr.subtypes.JoynrType;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class SubscriptionQos implements JoynrType {
    private static final Logger logger = LoggerFactory.getLogger(SubscriptionQos.class);

    private long expiryDate;
    private long publicationTtl;

    public static final int IGNORE_VALUE = -1;
    public static final long INFINITE_SUBSCRIPTION = Long.MAX_VALUE;
    private static final long MIN_PUBLICATION_TLL = 100L;
    private static final long MAX_PUBLICATION_TLL = 2592000000L; // 30 days

    protected static final long DEFAULT_PUBLICATION_TTL = 10000;

    protected SubscriptionQos() {
    }

    /**
     * 
     * @param expiryDate
     *            The expiryDate is the end date of the subscription. This value is provided in milliseconds (since
     *            1970-01-01T00:00:00.000).
     */
    public SubscriptionQos(long expiryDate) {
        this(expiryDate, DEFAULT_PUBLICATION_TTL);
    }

    /**
     * 
     * @param expiryDate
     *            : the end date of the subscription. This value is provided in milliseconds (since
     *            1970-01-01T00:00:00.000).
     * @param publicationTtl
     *            is the time-to-live for publication messages.<br>
     * <br>
     *            If a notification message can not be delivered within its time to live, it will be deleted from the
     *            system. This value is provided in milliseconds. <br>
     * <br>
     *            <b>Minimum and Maximum Values:</b>
     *            <ul>
     *            <li>minimum publicationTtl_ms = 100. Smaller values will be rounded up.
     *            <li>maximum publicationTtl_ms = 2 592 000 000 (30 days). Larger values will be rounded down.
     *            <li>defualt publicationTtl_ms = 10 000 (10 secs)
     *            </ul>
     */
    public SubscriptionQos(long expiryDate, long publicationTtl) {
        long now = System.currentTimeMillis();
        if (expiryDate <= now) {
            logger.error("Subscription ExpiryDate {} is in the past. Now: {}", expiryDate, now);
        }
        this.expiryDate = expiryDate;
        publicationTtl = publicationTtl < MIN_PUBLICATION_TLL ? MIN_PUBLICATION_TLL : publicationTtl;
        publicationTtl = publicationTtl > MAX_PUBLICATION_TLL ? MAX_PUBLICATION_TLL : publicationTtl;
        this.publicationTtl = publicationTtl;
    }

    /**
     * 
     * @return endDate_ms : the end date of the subscription. <br>
     *         This value is provided in milliseconds (since 1970-01-01T00:00:00.000).
     * 
     */
    public long getExpiryDate() {
        return expiryDate;
    }

    /**
     * 
     * @param expiryDate_ms
     *            is the end date of the subscription. <br>
     *            This value is provided in milliseconds (since 1970-01-01T00:00:00.000).
     * 
     */
    public void setExpiryDate(final long expiryDate_ms) {
        this.expiryDate = expiryDate_ms;
    }

    /**
     * Notification messages will be sent with this time-to-live.<br>
     * <br>
     * If a notification message can not be delivered within its time to live, it will be deleted from the system. This
     * value is provided in milliseconds.
     * 
     * @return publicationTtl_ms time-to-live in milliseconds.
     * 
     */
    public long getPublicationTtl() {
        return publicationTtl;
    }

    /**
     * Notification messages will be sent with this time-to-live. If a notification message can not be delivered within
     * its time to live, it will be deleted from the system. This value is provided in milliseconds.
     * 
     * @param publicationTtl_ms
     *            sets publicationTtl_ms time-to-live in milliseconds.
     *            <ul>
     *            <li>minimum publicationTtl_ms = 100. Smaller values will be rounded up.
     *            <li>maximum publicationTtl_ms = 2 592 000 000 (30 days). Larger values will be rounded down.
     *            </ul>
     * 
     */
    public void setPublicationTtl(final long publicationTtl_ms) {
        this.publicationTtl = publicationTtl_ms;
    }
}
