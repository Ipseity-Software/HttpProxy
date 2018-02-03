#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <mysql.h>

#define STRING_SIZE 50		// Max size of domain

char *resolve(const char *host)
{
	/* Settings */
	char *query = "SELECT ip FROM hosts WHERE domain=?;";
	char *mysql_host = "localhost";
	char *mysql_user = "HttpProxy";
	char *mysql_pass = "";
	char *mysql_db   = "HttpProxy";


	/* Debug message */
	printf("\t--In plugin mysql_resolve.resolve, host: %s\n", host);

	/* Connect to db*/
	MYSQL *con = mysql_init(NULL); // Init mysql
	if (con == NULL)
	{
		printf("\t--In plugin mysql_resolve.resolve, mysql_init() failed\n");
		return NULL;
	}

	if (mysql_real_connect(con, mysql_host, mysql_user, mysql_pass, mysql_db, 0, NULL, 0) == NULL) // Connect to mysql server
	{
		printf("\t--In plugin mysql_resolve.finish_with_error: %s\n", mysql_error(con));
		mysql_close(con);
		return NULL;
	}

	/* Prepare the statement */
	MYSQL_BIND bind[1];                          // Bind value storage
	MYSQL_STMT *stmt = mysql_stmt_init(con);     // Init statement

	if (mysql_stmt_prepare(stmt, query, strlen(query)))
	{
		printf("\t--ERROR: In plugin mysql_resolve.resolve, mysql_stmt_prepare(), SELECT failed\n");
		printf("\t-- %s\n", mysql_stmt_error(stmt));
		return NULL;
	}

	/* STRING PARAMETER */
	memset(bind, 0, sizeof(bind)); // Clear the bind
	unsigned long host_len = strlen(host);
	bind[0].buffer_type= MYSQL_TYPE_STRING;
	bind[0].buffer= (char *)host;
	bind[0].buffer_length= STRING_SIZE;
	bind[0].is_null= 0;
	bind[0].length= &host_len;

	if (mysql_stmt_bind_param(stmt, bind)) // Bind parameter
	{
		printf("\t--ERROR: In plugin mysql_resolve.resolve, mysql_stmt_bind_param()failed\n");
		printf("\t-- %s\n", mysql_stmt_error(stmt));
		return NULL;
	}

	mysql_stmt_execute(stmt); // Execute statement
	/* Result buffers */
	char str_data[STRING_SIZE];
	unsigned long length = 0;
	memset(bind, 0, sizeof(bind)); // Clear the bind
	bind[0].buffer_type= MYSQL_TYPE_STRING;
	bind[0].buffer=(char *) str_data;
	bind[0].buffer_length= STRING_SIZE;
	bind[0].is_null= 0;
	bind[0].length= &length;

	/* Bind the result buffers */
	if (mysql_stmt_bind_result(stmt, bind))
	{
		printf("\t--ERROR: In plugin mysql_resolve.resolve, mysql_stmt_bind_result() failed\n");
		printf("\t-- %s\n", mysql_stmt_error(stmt));
		return NULL;
	}

	/* Fetch one row */
	mysql_stmt_fetch(stmt);

	/* Close the statement */
	if (mysql_stmt_close(stmt))
	{
		printf("\t--ERROR: In plugin mysql_resolve.resolve, failed while closing the statement\n");
		printf("\t-- %s\n", mysql_error(con));
	}
	char *return_data = malloc (sizeof (char) * length);
	strcpy(return_data, str_data);
	return return_data;
}
