#pragma once
#define MainKey "Software\\��������\\CardProcess"
#define CONFIG  "C:\\WINDOWS\\system32\\"
#define MAXTRY   80
#define LICENSESTR	"��������.license"

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall InitCompanyList(const char *namelist);
int __stdcall CheckCompanyInList(const char *name);

int __stdcall CheckCounts(int maxtrys);

int __stdcall InitTimeLicense(const char *filename,const char *time);
int __stdcall CheckTimeLicense(const char *filename);

int __stdcall InitFullLicense(const char *filename);
int __stdcall CheckFullLicense(const char *filename);

#ifdef __cplusplus
}
#endif
