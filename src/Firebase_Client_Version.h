#ifndef FIREBASE_CLIENT_VERSION
#define FIREBASE_CLIENT_VERSION "4.3.19"
#define FIREBASE_CLIENT_VERSION_NUM 40319

/* The inconsistent file version checking to prevent mixed versions compilation. */
#define FIREBASE_CLIENT_VERSION_CHECK(ver) (ver == FIREBASE_CLIENT_VERSION_NUM)
#endif