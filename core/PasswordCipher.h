#ifndef PASSWORDCIPHER_H
#define PASSWORDCIPHER_H

#include <QtCore/QByteArray>
#include <QtCore/QString>

/*
  Blob layout (all big-endian where applicable):
    magic:  "PMGR"        (4 bytes)
    ver:    0x01          (1)
    kdf:    0x01          (1)  // 1=PBKDF2-SHA256
    aead:   0x01          (1)  // 1=AES-256-GCM
    slen:   0x10          (1)  // salt len (16)
    ivlen:  0x0C          (1)  // iv len  (12)
    tlen:   0x10          (1)  // tag len (16)
    rsvd:   0x00          (1)
    iters:  uint32_be     (4)  // PBKDF2 iterations
    salt:   slen bytes
    iv:     ivlen bytes
    ct:     N bytes
    tag:    tlen bytes
*/

class PasswordCipher
{
public:

  static const int kSaltLen  = 16;
  static const int kIVLen    = 12;  // GCM standard
  static const int kTagLen   = 16;
  static const int kKeyLen   = 32;  // AES-256
  static const int kPBKDF2Iters = 300000;

  static bool encrypt(const QString& passphrase,
                      const QByteArray& plaintext,
                      QByteArray& outBase64);

  static bool decrypt(const QString& passphrase,
                      const QByteArray& inBase64,
                      QByteArray& plaintext);

private:
  static bool kdf_pbkdf2_sha256(const QByteArray& passUtf8,
                                const QByteArray& salt,
                                int iters,
                                QByteArray& outKey32);

  static bool aes_gcm_encrypt(const QByteArray& key,
                              const QByteArray& iv,
                              const QByteArray& pt,
                              QByteArray& ct,
                              QByteArray& tag);

  static bool aes_gcm_decrypt(const QByteArray& key,
                              const QByteArray& iv,
                              const QByteArray& ct,
                              const QByteArray& tag,
                              QByteArray& pt);

  static void cleanse(QByteArray& buf);
};

#endif // PASSWORDCIPHER_H
