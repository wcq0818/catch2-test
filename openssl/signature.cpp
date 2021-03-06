#include "catch.hpp"
#include <iostream>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

void Exit(int step, std::string str, char* priv)
{
	printf("%d %s %s\n", step, str.c_str(), priv);
}

void tDigest(const EVP_MD *type)
{
	unsigned char md_value[EVP_MAX_MD_SIZE];//保存输出的摘要值的数组
	unsigned int md_len, i;
	EVP_MD_CTX mdctx; //EVP 消息摘要结构体
	char msg1[] = "TestMessage1";//待计算摘要的消息1
	char msg2[] = "TestMessage2"; //待计算摘要的消息2
	EVP_MD_CTX_init(&mdctx);//初始化摘要结构体
	EVP_DigestInit_ex(&mdctx, type, NULL); //设置摘要算法和密码算法引擎
	EVP_DigestUpdate(&mdctx, msg1, strlen(msg1)); //调用摘要Update计算msg1的摘要
	EVP_DigestUpdate(&mdctx, msg2, strlen(msg2)); //调用摘要Update计算msg2的摘要
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);//摘要结束，输出摘要值
	EVP_MD_CTX_cleanup(&mdctx); //释放内存
	printf("md_len = %02d ", md_len);
	for (i = 0; i < md_len; i++)
	{
		printf("0x%02x ", md_value[i]);
	}
	printf("\n");
}

TEST_CASE("md5_sha1_sha256_sha384", "[md5_sha1_sha256_sha384][.\hide]")
{
	OpenSSL_add_all_algorithms();
	tDigest(EVP_md5());
	tDigest(EVP_sha1());
	tDigest(EVP_sha256());
	tDigest(EVP_sha384());
}

/*
PKCS7Sign.cpp
Auth：Kagula
功能：调用OpenSSL实现数字签名功能例程（一）
环境：VS2008+SP1,OpenSSL1.0.1
*/

void InitOpenSSL()
{
	CRYPTO_malloc_init();
	/* Just load the crypto library error strings,
	* SSL_load_error_strings() loads the crypto AND the SSL ones */
	/* SSL_load_error_strings();*/
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
}

unsigned char * GetSign(char* keyFile, char* plainText, unsigned char* cipherText, unsigned int *cipherTextLen)
{
// 	FILE* fp = fopen(keyFile, "r");
// 	if (fp == NULL)
// 		return NULL;

	/* Read private key */

	RSA *rsa;
	EVP_PKEY *pkey = NULL;

	//  GENERATE KEY
	int bits = 4096;
	unsigned long exp = RSA_F4;
	rsa = RSA_generate_key(bits, exp, NULL, NULL);
	if (RSA_check_key(rsa) != 1)
	{
		Exit(1, "Error whilst checking key", keyFile);
	}
		
	pkey = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pkey, rsa);

// 	EVP_PKEY* pkey = NULL;
// 	pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
// 	fclose(fp);

	if (pkey == NULL)
	{
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	/* Do the signature */
	EVP_MD_CTX md_ctx;
	EVP_SignInit(&md_ctx, EVP_sha1());
	EVP_SignUpdate(&md_ctx, plainText, strlen(plainText));
	int err = EVP_SignFinal(&md_ctx, cipherText, cipherTextLen, pkey);

	if (err != 1) {
		ERR_print_errors_fp(stderr);
		return NULL;
	}

	EVP_PKEY_free(pkey);

	return cipherText;
}

bool VerifySign(char* certFile, unsigned char* cipherText, unsigned int cipherTextLen, char* plainText)
{
	/* Get X509 */
	FILE* fp = fopen(certFile, "r");
	if (fp == NULL)
		return false;
	X509* x509 = PEM_read_X509(fp, NULL, NULL, NULL);
	fclose(fp);

	if (x509 == NULL) {
		ERR_print_errors_fp(stderr);
		return false;
	}

	/* Get public key - eay */
	EVP_PKEY *pkey = X509_get_pubkey(x509);
	if (pkey == NULL) {
		ERR_print_errors_fp(stderr);
		return false;
	}

	/* Verify the signature */
	EVP_MD_CTX md_ctx;
	EVP_VerifyInit(&md_ctx, EVP_sha1());
	EVP_VerifyUpdate(&md_ctx, plainText, strlen((char*)plainText));
	int err = EVP_VerifyFinal(&md_ctx, cipherText, cipherTextLen, pkey);
	EVP_PKEY_free(pkey);

	if (err != 1) {
		ERR_print_errors_fp(stderr);
		return false;
	}
	return true;
}

int readKey()
{
	RSA *rsa;

	FILE *fp;
	if ((fp = fopen("E:\\public.pem", "r")) == NULL) {
		return -1;
	}
	rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
	RSA_free(rsa);
	fclose(fp);

	if ((fp = fopen("E:\\private.pem", "r")) == NULL) {
		return -1;
	}
	rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);

	RSA_free(rsa);
	return 0;
}

TEST_CASE("signature", "[signature][.\hide]")
{
	//readKey();

	char certFile[] = "E:\\client.crt";//含公匙
	char keyFile[] = "E:\\apri.pem";//含私匙

	char plainText[] = "I owe you...";//待签名的明文
	unsigned char cipherText[1024 * 4];
	unsigned int cipherTextLen;

	InitOpenSSL();

	memset(cipherText, 0, sizeof(cipherText));
	if (NULL == GetSign(keyFile, plainText, cipherText, &cipherTextLen))
	{
		printf("签名失败！\n");
	}

	if (false == VerifySign(certFile, cipherText, cipherTextLen, plainText))
	{
		printf("验证签名失败！\n");
	}


	printf("Signature Verified Ok.\n");
	::Sleep(10 * 1000);
}



TEST_CASE("GENERATE KEY", "[GENERATE KEY][]")
{
	InitOpenSSL();

	char priv_pem[] = "priv.pem";
	char pub_pem[] = "pub.pem";
	char pass[] = "Password";
	FILE *fp;
	int bits = 4096;
	unsigned long exp = RSA_F4;

	RSA *rsa;
	EVP_PKEY *pkey = NULL;

	//  GENERATE KEY
	rsa = RSA_generate_key(bits, exp, NULL, NULL);
	if (RSA_check_key(rsa) != 1)
		Exit(1, "Error whilst checking key", priv_pem);
	pkey = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pkey, rsa);

	//  WRITE ENCRYPTED PRIVATE KEY
	if (!(fp = fopen(priv_pem, "w")))
		Exit(2, "Error opening PEM file", priv_pem);
	if (!PEM_write_PrivateKey(fp, pkey, EVP_aes_256_cbc(), NULL, 0, NULL, pass))
		Exit(3, "Error writing PEM file", priv_pem);
	fclose(fp);

	//  WRITE PUBLIC KEY
	if (!(fp = fopen(pub_pem, "w")))
		Exit(4, "Error opening PEM file", pub_pem);
	if (!PEM_write_PUBKEY(fp, pkey))
		Exit(5, "Error writing PEM file", pub_pem);
	fclose(fp);
	// ------- End of key generation program -------

	// ------- Start of text signing program -------
	//  READ IN ENCRYPTED PRIVATE KEY
	if (!(fp = fopen(priv_pem, "r")))
		Exit(6, "Error reading encrypted private key file", priv_pem);
	if (!PEM_read_PrivateKey(fp, &pkey, NULL, pass))
		Exit(7, "Error decrypting private key file", priv_pem);
	fclose(fp);

	// Sign some text using the private key....

	//  FREE
	RSA_free(rsa);
}