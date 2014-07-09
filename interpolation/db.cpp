#include "db.h"

db::db(char* host, char* user, char* passwd, char* dbase)
{
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql,host,user,passwd,
                            dbase,0,NULL,0))
    {
        fprintf(stderr,"Error connectin ot database: %s\n",mysql_error(&mysql));
    }
    else fprintf(stderr,"Connected...\n");
}



db::~db()
{
    mysql_close(&mysql);
}

int db::select(char* sel)
{
    int t=mysql_real_query(&mysql,sel,(unsigned int)strlen(sel));
    if(t){
        fprintf(stderr,"Error making query: %s\n",
                mysql_error(&mysql));
        return 1;
    }
    // else fprintf(stderr,"Query made...\n");
    res = mysql_store_result(&mysql);
    return 0;
}

MYSQL_FIELD* db::get_fields()
{
    return(mysql_fetch_field(res));
}

unsigned int db::get_num_fields()
{
    return(mysql_num_fields(res));
}

unsigned int db::get_num_rows()
{
	return(mysql_num_rows(res));
}


char** db::get_row()
{
    return(mysql_fetch_row(res));
}

int db::clear_result()
{
    mysql_free_result(res);
}

db* get_dbpw()
{
  static bool __dbow__=false;
  static db* dbp;
  if(__dbow__==false){
    __dbow__=true;
    dbp=new db("mysql","gast","gast","clm");
  }
  return dbp;
}

db* get_dbpwth()
{
  static bool __dbow__=false;
  static db* dbp;
  if(__dbow__==false){
    __dbow__=true;
    dbp=new db("mysql","gast","gast","WETTREG2010");
  }
  return dbp;
}

db* get_dbpwth_org()
{
  static bool __dbow__=false;
  static db* dbp;
  if(__dbow__==false){
    __dbow__=true;
    dbp=new db("mysql","gast","gast","THUERINGEN_ORG");
  }
  return dbp;
}

db* get_dbps()
{
  static bool __dbow__=false;
  static db* dbp;
  if(__dbow__==false){
    __dbow__=true;
    dbp=new db("mysql","gast","gast","star2");
  }
  return dbp;
}

db* get_dbpl()
{
  static bool __dbow__=false;
  static db* dbp;
  if(__dbow__==false){
    __dbow__=true;
    dbp=new db("mysql","gast","gast","landcare");
  }
  return dbp;
}
