#ifndef FIREBASE_CLIENT_VERSION
#define FIREBASE_CLIENT_VERSION "4.4.3"
#define FIREBASE_CLIENT_VERSION_NUM 40403

/* The inconsistent file version checking to prevent mixed versions compilation. */
#define FIREBASE_CLIENT_VERSION_CHECK(ver) (ver == FIREBASE_CLIENT_VERSION_NUM)
#endif