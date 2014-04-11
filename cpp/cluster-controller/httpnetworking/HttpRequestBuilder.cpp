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
#include "cluster-controller/httpnetworking/HttpRequestBuilder.h"
#include "cluster-controller/httpnetworking/DefaultHttpRequest.h"
#include "joynr/exceptions.h"

#include <curl/curl.h>


namespace joynr {

using namespace joynr_logging;

Logger* HttpRequestBuilder::logger = Logging::getInstance()->getLogger("MSG", "HttpRequestBuilder");

HttpRequestBuilder::HttpRequestBuilder(const QString& url)
    : handle(NULL),
      headers(NULL),
      content(),
      built(false)
{
    handle = HttpNetworking::getInstance()->getCurlHandlePool()->getHandle(url);
    curl_easy_setopt(handle, CURLOPT_URL, url.toLatin1().data());
}

HttpRequestBuilder::~HttpRequestBuilder() {
    if(!built) {
        if(headers != 0) {
            curl_slist_free_all(headers);
        }
        HttpNetworking::getInstance()->getCurlHandlePool()->returnHandle(handle);
    }
}

HttpRequestBuilder* HttpRequestBuilder::withProxy(const QString& proxy) {
    curl_easy_setopt(handle, CURLOPT_PROXY, proxy.toLatin1().data());
    return this;
}


HttpRequestBuilder* HttpRequestBuilder::withDebug() {
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::postContent(const QByteArray& data) {
    content = data;
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::asPost() {
    curl_easy_setopt(handle, CURLOPT_POST, 1);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, 0);

    // Workaround for server behaviour described here:
    //  http://curl.haxx.se/mail/lib-2011-12/0348.html
    addHeader("Expect", "");
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::asDelete() {
    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::withTimeout_ms(long timeout_ms) {
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, timeout_ms);
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::withConnectTimeout_ms(long timeout_ms) {
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::acceptGzip() {
    curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "gzip");
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::withContentType(const QString& contentType) {
    addHeader("Content-Type", contentType);
    return this;
}

HttpRequestBuilder* HttpRequestBuilder::addHeader(const QString& name, const QString& value) {
    QString header(name + ": " + value);
    headers = curl_slist_append(headers, header.toLatin1().data());
    return this;
}

HttpRequest* HttpRequestBuilder::build() {
    if(built)   {
        LOG_WARN(logger, "The method build of HttpBuilder may be called only once on a specific instance. Throwing an Exception from worker thread.");
        throw JoynrException("The method build of HttpBuilder may be called only once on a specific instance");
    }
    built = true;
    return new DefaultHttpRequest(handle, content, headers);
}

} // namespace joynr
