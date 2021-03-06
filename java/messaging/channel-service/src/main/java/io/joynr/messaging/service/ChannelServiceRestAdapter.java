package io.joynr.messaging.service;

/*
 * #%L
 * joynr::java::messaging::channel-service
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

import static io.joynr.messaging.datatypes.JoynrMessagingErrorCode.JOYNRMESSAGINGERROR_CHANNELNOTSET;
import io.joynr.communications.exceptions.JoynrHttpException;
import io.joynr.messaging.info.Channel;
import io.joynr.messaging.info.ChannelInformation;

import java.util.List;
import java.util.concurrent.TimeUnit;

import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.HeaderParam;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.GenericEntity;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.atmosphere.annotation.Suspend;
import org.atmosphere.jersey.Broadcastable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.inject.Inject;

@Path("/channels")
/**
 * ChannelService is used by the messaging service of a cluster controller
 * to register for messages from other cluster controllers
 *
 * The following characters are allowed in the id
 * - upper and lower case characters
 * - numbers
 * - underscore (_) hyphen (-) and dot (.)
 */
public class ChannelServiceRestAdapter {

    private static final Logger log = LoggerFactory.getLogger(ChannelServiceRestAdapter.class);

    @Inject
    private ChannelService channelService;

    /**
     * A simple HTML list view of channels. A JSP is used for rendering.
     * 
     * @return a HTML list of available channels, and their resources (long poll
     *         connections) if connected.
     */
    @GET
    @Produces("application/json")
    public GenericEntity<List<ChannelInformation>> listChannels() {
        try {
            return new GenericEntity<List<ChannelInformation>>(channelService.listChannels()) {
            };
        } catch (Throwable e) {
            log.error("GET channels listChannels: error: {}", e.getMessage());
            throw new WebApplicationException(Status.INTERNAL_SERVER_ERROR);
        }
    }

    /**
     * Open a long poll channel for the given cluster controller.
     *  The channel is closed automatically by the server at
     * regular intervals to ensure liveliness.
     * 
     * @param ccid
     * @return new message(s), or nothing if the channel is closed by the servr
     */
    @GET
    @Path("/{ccid: [A-Z,a-z,0-9,_,\\-,\\.]+}")
    @Produces({ MediaType.APPLICATION_JSON })
    @Suspend(resumeOnBroadcast = true, period = ChannelServiceConstants.EXPIRE_TIME_CONNECTION, timeUnit = TimeUnit.SECONDS, contentType = MediaType.APPLICATION_JSON)
    public Broadcastable open(@PathParam("ccid") String ccid,
                              @HeaderParam("X-Cache-Index") Integer cacheIndex,
                              @HeaderParam(ChannelServiceConstants.X_ATMOSPHERE_TRACKING_ID) String atmosphereTrackingId) {

        try {
            return channelService.openChannel(ccid, cacheIndex, atmosphereTrackingId);
        } catch (WebApplicationException e) {
            throw e;
        } catch (Throwable e) {
            log.error("GET Channels open long poll ccid: error: {}", e.getMessage());
            throw new WebApplicationException(Status.INTERNAL_SERVER_ERROR);
        }
    }

    /**
     * HTTP POST to create a channel, returns location to new resource which can
     * then be long polled. Since the channel id may later change to be a UUID,
     * not using a PUT but rather POST with used id being returned
     * 
     */
    @POST
    @Produces({ MediaType.TEXT_PLAIN })
    public Response createChannel(@QueryParam("ccid") String ccid,
                                  @HeaderParam(ChannelServiceConstants.X_ATMOSPHERE_TRACKING_ID) String atmosphereTrackingId) {

        log.info("CREATE channel for channel ID: {}", ccid);

        if (ccid == null || ccid.isEmpty())
            throw new JoynrHttpException(Status.BAD_REQUEST, JOYNRMESSAGINGERROR_CHANNELNOTSET);

        Channel channel = channelService.getChannel(ccid);

        if (channel != null) {
            return Response.ok().header("Location", channel.getLocation().toString()).header("bp",
                                                                                             channel.getBounceProxy()
                                                                                                    .getId()).build();
        }

        // look for an existing bounce proxy handling the channel
        channel = channelService.createChannel(ccid, atmosphereTrackingId);

        // TODO error handling
        return Response.created(channel.getLocation()).header("bp", channel.getBounceProxy().getId()).build();
    }

    /**
     * Remove the channel for the given cluster controller.
     * 
     * @param ccid
     * @return
     */
    @DELETE
    @Path("/{ccid: [A-Z,a-z,0-9,_,\\-,\\.]+}")
    public Response deleteChannel(@PathParam("ccid") String ccid) {
        try {
            log.info("DELETE channel for cluster controller: {}", ccid);

            if (channelService.deleteChannel(ccid)) {
                return Response.ok().build();
            }
            return Response.noContent().build();

        } catch (WebApplicationException e) {
            throw e;
        } catch (Throwable e) {
            log.error("DELETE channel for cluster controller: error: {}", e.getMessage());
            throw new WebApplicationException(Status.INTERNAL_SERVER_ERROR);
        }
    }

}
