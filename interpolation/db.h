#include <string.h>
#include <stdlib.h>
#include <map>


// c++ work around to mysql
// version: 0.1 12.9.2003
// version: 0.2 15.3.2011
// Dr. Ralf Wieland

#ifndef __MYSQL__
#define __MYSQL__

#include <stdio.h>
#include <mysql/mysql.h>

class db{
 public:
    db(char*,char*,char*,char*);
    ~db();
    int select(char*);
    unsigned int get_num_rows();
    unsigned int get_num_fields();
    MYSQL_FIELD* get_fields();
    char** get_row();
    int clear_result();
 private:
    MYSQL mysql;
    MYSQL_RES *res;
};
                            


extern "C" MYSQL *  STDCALL mysql_init(MYSQL *mysql);
extern "C" MYSQL *  STDCALL mysql_real_connect(MYSQL *mysql, const char *host,
                                               const char *user,
                                               const char *passwd,
                                               const char *db,
                                               unsigned int port,
                                               const char *unix_socket,
                                               unsigned long clientflag);
extern "C" void     STDCALL mysql_close(MYSQL *sock);
extern "C" const char *   STDCALL mysql_error(MYSQL *mysql);
extern "C" unsigned int STDCALL mysql_num_fields(MYSQL_RES *res);
extern "C" int       STDCALL mysql_real_query(MYSQL *mysql, const char *q,
                                        unsigned long length);
extern "C" void      STDCALL mysql_free_result(MYSQL_RES *result);
extern "C" MYSQL_FIELD *   STDCALL mysql_fetch_field(MYSQL_RES *result);
extern "C" MYSQL_ROW       STDCALL mysql_fetch_row(MYSQL_RES *result);

db* get_dbpw();
db* get_dbpwth();
db* get_dbps();
db* get_dbpl();
db* get_dbpwth_org();

#endif

