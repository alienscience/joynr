package io.joynr.messaging.bounceproxy.controller;

/*
 * #%L
 * joynr::java::messaging::bounceproxy::bounceproxy-controller
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

import java.net.URI;
import java.util.List;

import com.google.inject.Inject;

import io.joynr.messaging.bounceproxy.controller.directory.BounceProxyDirectory;
import io.joynr.messaging.bounceproxy.controller.info.ControlledBounceProxyInformation;
import io.joynr.messaging.info.BounceProxyStatus;
import io.joynr.messaging.info.PerformanceMeasures;
import io.joynr.messaging.service.MonitoringService;

/**
 * Implementation of bounce proxy controller for monitoring service.
 * 
 * @author christina.strobel
 *
 */
public class MonitoringServiceImpl implements MonitoringService {

    @Inject
    private BounceProxyDirectory bounceProxyDirectory;

    @Override
    public List<String> getRegisteredBounceProxies() {
        return bounceProxyDirectory.getBounceProxyIds();
    }

    @Override
    public void register(String bpId, String urlForCc, String urlForBpc) {
        ControlledBounceProxyInformation bpInfo = new ControlledBounceProxyInformation(bpId,
                                                                                       URI.create(urlForCc),
                                                                                       URI.create(urlForBpc));
        bounceProxyDirectory.addBounceProxy(bpInfo);
    }

    @Override
    public void reset(String bpId, String urlForCc, String urlForBpc) {
        // TODO Auto-generated method stub

    }

    @Override
    public void updatePerformanceMeasures(String bpId, PerformanceMeasures performanceMeasures) {
        // TODO Auto-generated method stub

    }

    @Override
    public void updateStatus(String bpId, BounceProxyStatus status) {
        // TODO Auto-generated method stub

    }

    @Override
    public boolean isRegistered(String bpId) {
        // TODO Auto-generated method stub
        return false;
    }

}