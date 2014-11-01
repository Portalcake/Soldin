#ifndef __SOLIN_DATABASE_H__
#	define __SOLIN_DATABASE_H__
#	include <winsock2.h> 
#	include <mysql.h>
#	include <shared.h>
#	define MAX_CHARACTERS 32


typedef struct char_info_t {
	uint32_t id;
	char     name[33];
	uint32_t class_id;
	time_t   last_played;

	struct {
		uint16_t lv;
		uint32_t exp;
		uint32_t reqexp;
	} 
	lv_base;

	struct {
		uint16_t lv;
		uint32_t exp;
		uint32_t reqexp;
	} 
	lv_pvp;

	struct {
		uint16_t lv;
		uint32_t exp;
		uint32_t reqexp;
	} 
	lv_war;

	/* List of item ID's of the items that the character has equipped. */
	struct {
		uint32_t count;
		uint32_t list[32];
	}
	equipment;

	uint16_t rebirth_lv;
	uint16_t rebirth_count;
} CharacterInfo;


typedef struct account_info_t {
	uint32_t id;
	char     name[33];
	char     password[33];
	uint32_t maxchars;
	uint32_t gmlevel;

	/* List of the character licenses of the account. */
	struct {
		uint32_t count;
		uint32_t list[31];
	} licenses;

	/* List of characters owned by the account. */
	struct {
		uint32_t count;
		struct char_info_t list[MAX_CHARACTERS];
	} chars;
} AccountInfo;

extern MYSQL *g_conn;

bool               db_connect(const char *host, const char *user, const char *passwd, const char *db, uint16_t port);
AccountInfo       *db_open_account(const char *username);
uint32_t           db_get_character_list(uint32_t acc_id, CharacterInfo *list);
CharacterInfo     *db_get_character(uint32_t char_id, CharacterInfo *dest);
inline const char *db_error() { return mysql_error(g_conn); }
inline uint32_t    db_errno() { return mysql_errno(g_conn); }
bool               db_character_exists(const char *name);
uint32_t           db_character_create(uint32_t account_id, uint32_t class_id, const char *name);
void               db_character_delete(uint32_t char_id);

#endif /* __SOLIN_DATABASE_H__ */
