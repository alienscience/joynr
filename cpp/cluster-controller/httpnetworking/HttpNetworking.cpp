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
#include "cluster-controller/httpnetworking/HttpNetworking.h"

#include "cluster-controller/httpnetworking/DefaultHttpRequest.h"
#include "cluster-controller/httpnetworking/HttpRequestBuilder.h"
#include "cluster-controller/httpnetworking/HttpResult.h"

#include "cluster-controller/httpnetworking/CurlHandlePool.h"

#include <curl/curl.h>
#include <QString>
#include <QByteArray>
#include <QMultiMap>

namespace joynr {

HttpNetworking* HttpNetworking::httpNetworking = new HttpNetworking();

HttpNetworking::HttpNetworking() :
    curlHandlePool(NULL),
    proxy(),
    certificateAuthority(),
    clientCertificate(),
    clientCertificatePassword(),
    httpDebug(false)
{
    curl_global_init(CURL_GLOBAL_ALL);
    //curlHandlePool = new DefaultCurlHandlePool;
    //curlHandlePool = new AlwaysNewCurlHandlePool;
    curlHandlePool = new PerThreadCurlHandlePool;
}

HttpNetworking* HttpNetworking::getInstance() {
    return httpNetworking;
}

HttpRequestBuilder* HttpNetworking::createRequestBuilder(const QString& url) {
    HttpRequestBuilder* requestBuilder = new HttpRequestBuilder(url);
    if(!proxy.isEmpty()) {
        requestBuilder->withProxy(proxy);
    }
    if (httpDebug) {
        requestBuilder->withDebug();
    }

    // Check for HTTPS options
    if (!certificateAuthority.isEmpty()) {
        requestBuilder->withCertificateAuthority(certificateAuthority);
    }

    if (!clientCertificate.isEmpty()) {
        requestBuilder->withClientCertificate(clientCertificate);
    }

    if (!clientCertificatePassword.isEmpty()) {
        requestBuilder->withClientCertificatePassword(clientCertificatePassword);
    }

    return requestBuilder;
}

IHttpGetBuilder* HttpNetworking::createHttpGetBuilder(const QString& url) {
    return createRequestBuilder(url);
}

IHttpDeleteBuilder* HttpNetworking::createHttpDeleteBuilder(const QString& url) {
    return createRequestBuilder(url)->asDelete();
}

IHttpPostBuilder* HttpNetworking::createHttpPostBuilder(const QString& url) {
    return createRequestBuilder(url)->asPost();
}

IHttpPostBuilder::~IHttpPostBuilder()
{
}

void HttpNetworking::setGlobalProxy(const QString& proxy) {
    this->proxy = proxy;
}

void HttpNetworking::setHTTPDebugOn() {
    this->httpDebug = true;
}

void HttpNetworking::setCertificateAuthority(const QString& certificateAuthority)
{
    this->certificateAuthority = certificateAuthority;
}

void HttpNetworking::setClientCertificate(const QString& clientCertificate)
{
    this->clientCertificate = clientCertificate;
}

void HttpNetworking::setClientCertificatePassword(const QString& clientCertificatePassword)
{
    this->clientCertificatePassword = clientCertificatePassword;
}


ICurlHandlePool* HttpNetworking::getCurlHandlePool() {
    return curlHandlePool;
}

ICurlHandlePool::~ICurlHandlePool()
{

}

} // namespace joynr
