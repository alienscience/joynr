package io.joynr.integration.util;

public class SSLSettings {
    private final String keyStorePath;
    private final String trustStorePath;
    private final String keyStorePassword;
    private final String trustStorePassword;

    public SSLSettings(String keyStorePath, String trustStorePath, String keyStorePassword, String trustStorePassword) {
        this.keyStorePath = keyStorePath;
        this.trustStorePath = trustStorePath;
        this.keyStorePassword = keyStorePassword;
        this.trustStorePassword = trustStorePassword;
    }

    public String getKeyStorePath() {
        return keyStorePath;
    }

    public String getTrustStorePath() {
        return trustStorePath;
    }

    public String getKeyStorePassword() {
        return keyStorePassword;
    }

    public String getTrustStorePassword() {
        return trustStorePassword;
    }

}