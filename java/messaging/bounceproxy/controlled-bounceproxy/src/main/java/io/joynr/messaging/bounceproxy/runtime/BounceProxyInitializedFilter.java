package io.joynr.messaging.bounceproxy.runtime;

/*
 * #%L
 * joynr::java::messaging::bounceproxy::controlled-bounceproxy
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

import io.joynr.exceptions.JoynrException;
import io.joynr.messaging.bounceproxy.monitoring.MonitoringServiceClient;

import java.io.IOException;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;

import com.google.inject.Inject;
import com.google.inject.Singleton;

/**
 * Servlet filter that only lets requests pass if the bounce proxy was
 * initialized correctly.
 * 
 * @author christina.strobel
 * 
 */
@Singleton
public class BounceProxyInitializedFilter implements Filter {

    @Inject
    private MonitoringServiceClient monitoringServiceClient;

    @Override
    public void init(FilterConfig filterConfig) throws ServletException {
    }

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException,
                                                                                             ServletException {

        // first check whether we are ready to accept events
        if (monitoringServiceClient.hasReportedStartup()) {
            // forward request
            chain.doFilter(request, response);
        }

        // block request
        throw new JoynrException("Bounce proxy is not ready to accept requests.");
    }

    @Override
    public void destroy() {
    }
}