#define _GNU_SOURCE
#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/ssl.h>
extern char * __progname;
static int (*hook_open)(const char *path , int flags, mode_t mode)= NULL;
static int (*hook_open64)(const char *path , int flags, mode_t mode)= NULL;
static FILE *(*hook_fopen)(const char *path, const char *mode) = NULL;
static int (*hook_unlinkat)(int dirfd, const char *path, int flags) = NULL;
static int (*hook_unlink)(const char *path) = NULL;


static int (*hook_ssl_write)(SSL *ssl, const void *buf, int num) = NULL;
static int (*hook_ssl_read)(SSL *ssl, void *buf, int num) = NULL;


#define MAX 0x100
#define DEFAULT_FILTER "/tmp/"

char FILTER[MAX] ="/tmp/";
FILE * cf_file = NULL;


void getNamePID(int pid, char *pName)
{
   int fp = 0;
   char buff[MAX]={0,};
   snprintf(buff,MAX,"/proc/%d/cmdline",pid);
   fp = hook_open(buff,0,0);
   read(fp,(char *)pName,MAX);
   close(fp);
}
void CheckConfig()
{
   if ( cf_file == NULL )
   {
      cf_file = hook_fopen("/tmp/hook.ini","r");
      if(cf_file) fscanf(cf_file,"%s",FILTER);
      if(strlen(FILTER)<1) strncpy(FILTER,DEFAULT_FILTER,sizeof(FILTER));
   }

}

void PrintLog(char * real_path)
{
   char pName[MAX] = {0,};
   FILE *fp = NULL;
   if( strstr(real_path,FILTER) > 0 )
   {
      if( fp = hook_fopen("/tmp/file_log.txt", "a+") )
      {
         getNamePID(getppid(),pName);
         fprintf(fp,"[-] Caller-> pName:[%s]:[%d],ppName:[%s]:[%d]->%s\n" \
            ,__progname, getpid(),pName,getppid(),real_path );
         fclose(fp);
      }
      
   }

}

void myPrintLog(char * text)
{   
   FILE *fp = NULL;
   if( fp = hook_fopen("/tmp/file_log.txt", "a+") )
      {
         fprintf(fp,"[SSL]->\n%s\n" , text );
         fclose(fp);
      }

}

int unlinkat(int dirfd, const char *path, int flags)
{
   if (hook_unlinkat == NULL)hook_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
   char *real_path = realpath(path,0);
   if ( real_path )
   {
      CheckConfig();
      PrintLog(real_path);
   }
   return hook_unlinkat(dirfd, path, flags);
}

int unlink(const char *path)
{
   if (hook_unlink   == NULL) hook_unlink = dlsym(RTLD_NEXT, "unlink");
   char *real_path = realpath(path,0);
   if ( real_path )
   {
      CheckConfig();
      PrintLog( real_path );

   }
   return hook_unlink(path);
}
int open(const char * path, int flags, mode_t mode)
{
   if (hook_open    == NULL) hook_open = dlsym(RTLD_NEXT, "open");
   int hook_ret    = hook_open(path,flags,mode);
   char *real_path = realpath(path,0);
   if ( real_path )
   {
      CheckConfig();
      PrintLog( real_path );
   }
   return hook_ret;
}
int open64(const char * path, int flags, mode_t mode)
{
   if (hook_open64 == NULL) hook_open64 = dlsym(RTLD_NEXT, "open64");
   int hook_ret    = hook_open64(path,flags,mode);
   char *real_path = realpath(path,0);
   if ( real_path )
   {
      CheckConfig();
      PrintLog( real_path );
   }
   return hook_ret; 
}

FILE * fopen(const char *path, const char *mode)
{
   if (hook_fopen    == NULL) hook_fopen = dlsym(RTLD_NEXT, "fopen");
   FILE *hook_ret    = hook_fopen(path,mode);
   char *real_path = realpath(path,0);
   if ( real_path )
   {
      CheckConfig();
      PrintLog( real_path );
   }
   return hook_ret; 
}

int SSL_write(SSL *ssl, const void *buf, int num)
{   
   if(hook_ssl_write == NULL) hook_ssl_write = dlsym(RTLD_NEXT , "SSL_write");
   int ret = hook_ssl_write(ssl, buf, num);

   if( buf != NULL)
   {
      myPrintLog((char *) buf);
   }

	return ret;

}


int SSL_read(SSL *ssl, void *buf, int num)
{

   if(hook_ssl_read == NULL) hook_ssl_write = dlsym(RTLD_NEXT , "SSL_read");
   int ret = hook_ssl_read(ssl, buf, num);

   if( buf != NULL)
   {
      myPrintLog((char *) buf);
   }

	return ret;

}



void __attribute__ ((constructor)) before_load(void)
{
   if (hook_open    == NULL) hook_open    = dlsym(RTLD_NEXT, "open");
   if (hook_open64 == NULL) hook_open64    = dlsym(RTLD_NEXT, "open64");
   if (hook_fopen    == NULL) hook_fopen    = dlsym(RTLD_NEXT, "fopen");
   if (hook_unlink   == NULL) hook_unlink    = dlsym(RTLD_NEXT, "unlink");
   if (hook_unlinkat == NULL)hook_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
   if (hook_ssl_write == NULL) hook_ssl_write = dlsym(RTLD_NEXT , "SSL_write");
   if (hook_ssl_read == NULL) hook_ssl_read = dlsym(RTLD_NEXT , "SSL_read");

}
