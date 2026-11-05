plugins { id("com.android.application") }

android {
    namespace = "com.aftereffects.mobile"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.aftereffects.mobile"
        minSdk = 26
        targetSdk = 28
        versionCode = 1
        versionName = "0.1.0"
        ndk { abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64") }
        externalNativeBuild {
            cmake { cppFlags += listOf("-std=c++17", "-fexceptions", "-frtti") }
        }
    }

    ndkVersion = "27.0.12077973"
    externalNativeBuild { cmake { path = file("src/main/cpp/CMakeLists.txt"); version = "3.22.1" } }
}
