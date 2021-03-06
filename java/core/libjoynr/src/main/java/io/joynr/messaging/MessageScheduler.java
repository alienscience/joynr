package io.joynr.messaging;

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

import io.joynr.exceptions.JoynrChannelMissingException;
import io.joynr.exceptions.JoynrCommunicationException;
import io.joynr.exceptions.JoynrMessageNotSentException;
import io.joynr.exceptions.JoynrSendBufferFullException;
import io.joynr.exceptions.JoynrShutdownException;
import io.joynr.exceptions.JoynrTimeoutException;
import io.joynr.messaging.datatypes.JoynrMessagingError;
import io.joynr.messaging.datatypes.JoynrMessagingErrorCode;
import io.joynr.messaging.httpoperation.FailureAction;
import io.joynr.messaging.httpoperation.HttpConstants;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.util.List;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nullable;

import joynr.types.ChannelUrlInformation;

import org.apache.http.HttpEntity;
import org.apache.http.StatusLine;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.config.RequestConfig.Builder;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;
import org.apache.http.util.EntityUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import com.google.inject.Inject;
import com.google.inject.name.Named;

/**
 * The MessageScheduler queues message post requests in a single threaded executor. The executor is blocked until the
 * connection is established, from there on the request is async. If there are already too much connections open, the
 * executor is blocked until one of the connections is closed. Resend attempts are scheduled by a cached thread pool
 * executor.
 */
@edu.umd.cs.findbugs.annotations.SuppressWarnings(value = "JLM_JSR166_UTILCONCURRENT_MONITORENTER", justification = "ensure that now new messages are scheduled when scheduler is shuting down")
public class MessageScheduler {
    private static final int DELAY_RECEIVER_NOT_STARTED_MS = 100;
    private static final long TERMINATION_TIMEOUT = 5000;
    private static final long SCHEDULER_KEEP_ALIVE_TIME = 100;
    private final ScheduledThreadPoolExecutor scheduler;
    private static final Logger logger = LoggerFactory.getLogger(MessageScheduler.class);
    private CloseableHttpClient httpclient;
    private HttpConstants httpConstants;
    private RequestConfig defaultRequestConfig;
    private ObjectMapper objectMapper;
    private final LocalChannelUrlDirectoryClient channelUrlClient;

    @Inject
    public MessageScheduler(CloseableHttpClient httpclient,
                            @Named(ConfigurableMessagingSettings.PROPERTY_MESSAGING_MAXIMUM_PARALLEL_SENDS) int maximumParallelSends,
                            LocalChannelUrlDirectoryClient localChannelUrlClient,
                            RequestConfig defaultRequestConfig,
                            HttpConstants httpConstants,
                            ObjectMapper objectMapper) {
        this.httpclient = httpclient;
        channelUrlClient = localChannelUrlClient;
        this.defaultRequestConfig = defaultRequestConfig;
        this.httpConstants = httpConstants;
        this.objectMapper = objectMapper;

        ThreadFactory schedulerNamedThreadFactory = new ThreadFactoryBuilder().setNameFormat("joynr.MessageScheduler-scheduler-%d")
                                                                              .build();
        scheduler = new ScheduledThreadPoolExecutor(maximumParallelSends, schedulerNamedThreadFactory);
        scheduler.setKeepAliveTime(SCHEDULER_KEEP_ALIVE_TIME, TimeUnit.SECONDS);
        scheduler.allowCoreThreadTimeOut(true);
    }

    public synchronized void scheduleMessage(final MessageContainer messageContainer,
                                             long delay_ms,
                                             final FailureAction failureAction,
                                             final MessageReceiver messageReceiver) {
        logger.trace("scheduleMessage messageId: {} channelId {}",
                     messageContainer.getMessageId(),
                     messageContainer.getChannelId());
        // check if messageReceiver is ready to receive replies otherwise delay request by at least 100 ms
        if (!messageReceiver.isChannelCreated()) {
            delay_ms = delay_ms > DELAY_RECEIVER_NOT_STARTED_MS ? delay_ms : DELAY_RECEIVER_NOT_STARTED_MS;
        }

        synchronized (scheduler) {
            if (scheduler.isShutdown()) {
                JoynrShutdownException joynrShutdownEx = new JoynrShutdownException("MessageScheduler is shutting down already. Unable to send message [messageId: "
                        + messageContainer.getMessageId() + "].");
                logger.error("scheduler already shutting down", joynrShutdownEx);
                failureAction.execute(joynrShutdownEx);
                throw joynrShutdownEx;
            }

            try {
                scheduler.schedule(new Runnable() {
                    public void run() {
                        if (!messageReceiver.isChannelCreated()) {
                            scheduleMessage(messageContainer,
                                            DELAY_RECEIVER_NOT_STARTED_MS,
                                            failureAction,
                                            messageReceiver);
                            logger.debug("Creation of Channel for channelId {} is still ongoing. Sending messages now could lead to lost replies - delaying sending messageId {}",
                                         messageReceiver.getChannelId(),
                                         messageContainer.getMessageId());
                            return;
                        }

                        sendMessage(messageContainer, failureAction);
                    }
                },
                                   delay_ms,
                                   TimeUnit.MILLISECONDS);
            } catch (RejectedExecutionException e) {
                logger.error("Execution rejected while scheduling SendSerializedMessageRequest ", e);
                throw new JoynrSendBufferFullException(e);
            }
        }
    }

    private void sendMessage(final MessageContainer messageContainer, final FailureAction failureAction) {
        logger.trace("SEND messageId: {} channelId: {}",
                     messageContainer.getMessageId(),
                     messageContainer.getChannelId());

        HttpContext context = new BasicHttpContext();

        String channelId = messageContainer.getChannelId();
        String messageId = messageContainer.getMessageId();

        if (messageContainer.isExpired()) {
            logger.error("SEND executionQueue.run channelId: {}, messageId: {} TTL expired: ",
                         messageId,
                         messageContainer.getExpiryDate());
            failureAction.execute(new JoynrTimeoutException(messageContainer.getExpiryDate()));
            return;
        }

        // execute http command to send
        CloseableHttpResponse response = null;
        try {

            String serializedMessage = messageContainer.getSerializedMessage();
            final String sendUrl = getSendUrl(messageContainer.getChannelId());
            logger.debug("SENDING message channelId: {}, messageId: {} toUrl: {}", new String[]{ channelId, messageId,
                    sendUrl });
            if (sendUrl == null) {
                logger.error("SEND executionQueue.run channelId: {}, messageId: {} No channelId found",
                             messageId,
                             messageContainer.getExpiryDate());
                failureAction.execute(new JoynrMessageNotSentException("no channelId found"));
                return;
            }

            HttpPost httppost = new HttpPost(sendUrl);
            httppost.addHeader(httpConstants.getHEADER_CONTENT_TYPE(), httpConstants.getAPPLICATION_JSON()
                    + ";charset=UTF-8");
            httppost.setEntity(new StringEntity(serializedMessage, "UTF-8"));

            // Clone the default config
            Builder requestConfigBuilder = RequestConfig.copy(defaultRequestConfig);
            requestConfigBuilder.setConnectionRequestTimeout(httpConstants.getSEND_MESSAGE_REQUEST_TIMEOUT());
            httppost.setConfig(requestConfigBuilder.build());

            response = httpclient.execute(httppost, context);

            StatusLine statusLine = response.getStatusLine();
            int statusCode = statusLine.getStatusCode();
            String statusText = statusLine.getReasonPhrase();

            switch (statusCode) {
            case HttpURLConnection.HTTP_CREATED:
                logger.debug("SEND to ChannelId: {} messageId: {} completed successfully", channelId, messageId);
                break;
            case HttpURLConnection.HTTP_BAD_REQUEST:
                try {
                    HttpEntity entity = response.getEntity();
                    if (entity == null) {
                        logger.error("SEND to ChannelId: {} messageId: {} completed in error. No further reason found in message body",
                                     channelId,
                                     messageId);
                        return;
                    }
                    String body = EntityUtils.toString(entity, "UTF-8");

                    JoynrMessagingError error = objectMapper.readValue(body, JoynrMessagingError.class);
                    JoynrMessagingErrorCode joynrMessagingErrorCode = JoynrMessagingErrorCode.getJoynrMessagingErrorCode(error.getCode());
                    logger.error(error.toString());
                    switch (joynrMessagingErrorCode) {
                    case JOYNRMESSAGINGERROR_CHANNELNOTFOUND:
                        failureAction.execute(new JoynrChannelMissingException("Channel does not exist. Status: "
                                + statusCode + " error: " + error.getCode() + "reason:" + error.getReason()));
                        break;
                    default:
                        failureAction.execute(new JoynrCommunicationException("Http Error while communicating: "
                                + statusText + body + " error: " + error.getCode() + "reason:" + error.getReason()));
                        break;
                    }
                } catch (Exception e) {
                    failureAction.execute(new JoynrCommunicationException("Http Error while communicating: "
                            + statusText));

                }
            default:
                break;
            }
        } catch (Exception e) {
            logger.trace("SEND error channelId: {}, messageId: {} error: {}", new Object[]{ channelId, messageId,
                    e.getMessage() });
        } finally {
            if (response != null) {
                try {
                    response.close();
                } catch (IOException e) {
                }
            }
        }
    }

    @Nullable
    private String getSendUrl(String channelId) {

        ChannelUrlInformation channelUrlInfo = channelUrlClient.getUrlsForChannel(channelId);
        String url = null;

        List<String> urls = channelUrlInfo.getUrls();
        if (!urls.isEmpty()) {
            url = urls.get(0) + "message/"; // TODO handle trying multiple channelUrls
        }

        return url;
    }

    /**
     * Stops the scheduler thread pool and the execution thread.
     * 
     * @throws InterruptedException
     */
    public synchronized void shutdown() throws InterruptedException {
        synchronized (scheduler) {
            scheduler.shutdown();
        }

        // TODO serialize messages that could not be resent because of shutdown? Or somehow notify sender?
        // List<Runnable> awaitingScheduling = scheduler.shutdownNow();
        // List<Runnable> awaitingResend = executionQueue.shutdownNow();
        scheduler.awaitTermination(TERMINATION_TIMEOUT, TimeUnit.MILLISECONDS);
    }

}
