/*
 * Seven Kingdoms 2: The Fryhtan War
 *
 * Copyright 1999 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : OSPRTRES.CPP
//Description : Object Sprite resource

#include <osprtres.h>
#include <all.h>
#include <osys.h>
#include <oworld.h>
#include <oworldmt.h>
#include <ogameset.h>
#include <oweather.h>
#include <ovga.h>
#include <ocoltbl.h>
#include <ogame.h>


//-------- define file name -----------//

#define SPRITE_DB				"SPRITE"
#define SPRITE_ACTION_DB   "SACTION"
#define SUB_SPRITE_DB		"SUB_SPR"


//-------- Begin of function SpriteRes::init ---------//

void SpriteRes::init()
{
	deinit();

	// open sprite palette resource
	res_pal.init_imported(DIR_RES"PAL_SPR.RES", 1);  // 1-read all into buffer

	load_sprite_info();
	load_sub_sprite_info();
	fix_sprite_info();

	init_flag=1;
}
//--------- End of function SpriteRes::init ----------//


//-------- Begin of function SpriteRes::deinit ---------//

void SpriteRes::deinit()
{
	if( init_flag )
	{
		delete[] sprite_info_array;
		mem_del(sub_sprite_info_array);

		res_pal.deinit();

		init_flag=0;
	}
}
//--------- End of function SpriteRes::deinit ----------//


//-------- Begin of function SpriteRes::load_sprite_info ---------//

void SpriteRes::load_sprite_info()
{
	Database   	    *dbSprite 	 = game_set.open_db(SPRITE_DB);
	SpriteRec  	    *spriteRec;
	SpriteInfo 	    *spriteInfo;
	SpriteActionRec *spriteActionRec;
	SpriteMove      *spriteMove;
	SpriteAttack    *spriteAttack;
	int		  	    i, j, actionRecno, dirId;

	sprite_info_count = dbSprite->rec_count();
	sprite_info_array = new SpriteInfo[sprite_info_count];

	memset( sprite_info_array, 0, sizeof(SpriteInfo)*sprite_info_count );

	short* first_dir_recno_array = (short*) mem_add( sizeof(short) * sprite_info_count );	// allocate temporary arrays for temporary storage
	short* dir_count_array		  = (short*) mem_add( sizeof(short) * sprite_info_count );

	//------------ read in sprite info -------------//

	for( i=0 ; i<sprite_info_count ; i++ )
	{
		spriteRec  = (SpriteRec*) dbSprite->read(i+1);
		spriteInfo = sprite_info_array+i;

		m.rtrim_fld( spriteInfo->sprite_code, spriteRec->sprite_code, spriteRec->CODE_LEN );

		spriteInfo->sprite_type = spriteRec->sprite_type;

		if(spriteInfo->sprite_type == ' ')
			spriteInfo->sprite_type = 0;

		spriteInfo->sprite_sub_type = spriteRec->sprite_sub_type;

		if(spriteInfo->sprite_sub_type == ' ')
			spriteInfo->sprite_sub_type = 0;

		if( spriteRec->need_turning != ' ' )
			spriteInfo->need_turning = spriteRec->need_turning-'0';

		spriteInfo->turn_resolution = m.atoi(spriteRec->turn_resolution, spriteRec->TURN_RES_LEN);
		spriteInfo->loc_width  = m.atoi(spriteRec->loc_width , spriteRec->SPRITE_PARA_LEN);
		spriteInfo->loc_height = m.atoi(spriteRec->loc_height, spriteRec->SPRITE_PARA_LEN);
		spriteInfo->max_height = m.atoi(spriteRec->max_height, spriteRec->SPRITE_MAX_HEIGHT_LEN);
						
		spriteInfo->max_height += -spriteInfo->loc_width*LOCATE_WIDTH/2 - (-spriteInfo->loc_width*ZOOM_LOC_X_WIDTH/2 + -spriteInfo->loc_height*ZOOM_LOC_Y_WIDTH/2);

		spriteInfo->max_bitmap_height = m.atoi(spriteRec->max_bitmap_height, spriteRec->SPRITE_MAX_HEIGHT_LEN);

		spriteInfo->speed      		 = m.atoi(spriteRec->speed     		, spriteRec->SPRITE_PARA_LEN);
		spriteInfo->max_speed       = m.atoi(spriteRec->speed          , spriteRec->SPRITE_PARA_LEN);
		spriteInfo->frames_per_step = m.atoi(spriteRec->frames_per_step, spriteRec->SPRITE_PARA_LEN);

		spriteInfo->max_rain_slowdown = m.atoi(spriteRec->max_rain_slowdown, spriteRec->SPRITE_PARA_LEN);
		spriteInfo->max_snow_slowdown = m.atoi(spriteRec->max_snow_slowdown, spriteRec->SPRITE_PARA_LEN);
		spriteInfo->lightning_damage = m.atoi(spriteRec->lightning_damage, spriteRec->DAMAGE_LEN);

		spriteInfo->bullet_delay = m.atoi(spriteRec->bullet_delay, spriteRec->DAMAGE_LEN);
		
		if( spriteRec->remap_bitmap_flag == '\0' 
			|| spriteRec->remap_bitmap_flag == ' '
			|| spriteRec->remap_bitmap_flag == '0' )
			spriteInfo->remap_bitmap_flag = 0;
		else
			spriteInfo->remap_bitmap_flag = 1;

		if( spriteRec->pal_file_name[0] == ' ' || spriteRec->pal_file_name[0] == '\0' )
		{
			spriteInfo->pal_ptr = NULL;
		}
		else
		{
			long paletteOffset;
			memcpy(&paletteOffset, spriteRec->pal_offset, sizeof(long));
			spriteInfo->pal_ptr = 8+(BYTE *)res_pal.read_imported(paletteOffset);	// skip 8 bytes header
		}
		spriteInfo->palw_ptr = NULL;

		first_dir_recno_array[i] = m.atoi(spriteRec->first_move_recno, spriteRec->RECNO_LEN);
		dir_count_array[i] 		 = m.atoi(spriteRec->move_count      , spriteRec->COUNT_LEN);
	}

	//------- read in sprite action info ---------//

	Database *dbSpriteMove = game_set.open_db(SPRITE_ACTION_DB);

	for( i=0 ; i<sprite_info_count ; i++ )
	{
		spriteInfo = sprite_info_array+i;

		for( j=0, actionRecno=first_dir_recno_array[i] ; j<dir_count_array[i] ; j++, actionRecno++ )
		{
			spriteActionRec = (SpriteActionRec*) dbSpriteMove->read(actionRecno);

			dirId = m.atoi(spriteActionRec->dir_id, spriteActionRec->DIR_ID_LEN);

			//--------- move motion --------//

			if( spriteActionRec->action[0] == 'M' )
			{
				spriteMove = spriteInfo->move_array+dirId;

				spriteMove->first_frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
				spriteMove->frame_count 		= m.atoi(spriteActionRec->frame_count, spriteActionRec->COUNT_LEN);

				//--- the first movement frame is the default stop frame ---//

				if( spriteInfo->stop_array[dirId].frame_recno == 0)
				{
					spriteInfo->stop_array[dirId].frame_recno = spriteMove->first_frame_recno;
					spriteInfo->stop_array[dirId].frame_count = 1;
				}
			}

			//-------- attacking motion or weapon motion --------//

			else if( spriteActionRec->action[0] == 'A' )
			{
				err_when( spriteActionRec->action[1] < '1' || spriteActionRec->action[1] > '9' );

				spriteAttack = spriteInfo->attack_array[spriteActionRec->action[1]-'1'] + dirId;

				spriteAttack->first_frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
				spriteAttack->frame_count 		  = m.atoi(spriteActionRec->frame_count	   , spriteActionRec->COUNT_LEN);
			}

			//--------- stop bitmap ---------//

			else if( spriteActionRec->action[0] == 'S' )
			{
				spriteInfo->stop_array[dirId].frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
				spriteInfo->stop_array[dirId].frame_count = m.atoi(spriteActionRec->frame_count, spriteActionRec->COUNT_LEN);
			}

			//-------- dying motion ---------//

			else if( spriteActionRec->action[0] == 'D' )
			{
				spriteInfo->die.first_frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
				spriteInfo->die.frame_count 		  = m.atoi(spriteActionRec->frame_count	  , spriteActionRec->COUNT_LEN);
			}

			//--------- guarding motion --------//

			else if( spriteActionRec->action[0] == 'G' )
			{
				if( spriteActionRec->action[1] == 'M' )
				{
					// moving guard
					SpriteGuardMove *spriteGuardMove = spriteInfo->guard_move_array+dirId;
					spriteGuardMove->first_frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
					spriteGuardMove->frame_count 		= m.atoi(spriteActionRec->frame_count, spriteActionRec->COUNT_LEN);

					// set can_guard_flag
					spriteInfo->can_guard_flag |= 2;
				}
				else
				{
					// standing guard
					SpriteGuardStop *spriteGuardStop = spriteInfo->guard_stop_array+dirId;
					spriteGuardStop->first_frame_recno = m.atoi(spriteActionRec->first_frame_recno, spriteActionRec->RECNO_LEN);
					spriteGuardStop->frame_count 		= m.atoi(spriteActionRec->frame_count, spriteActionRec->COUNT_LEN);

					// set can_guard_flag
					spriteInfo->can_guard_flag |= 1;
				}
			}
		}
	}

	//----------- delete temp arrays -------------//

	mem_del( first_dir_recno_array );
	mem_del( dir_count_array );
}
//-------- End of function SpriteRes::load_sprite_info ---------//


//-------- Begin of function SpriteRes::load_sub_sprite_info ---------//

void SpriteRes::load_sub_sprite_info()
{
	//------------ read in sub sprite info -------------//

	Database *dbSubSprite = game_set.open_db(SUB_SPRITE_DB);

	int subSpriteCount = dbSubSprite->rec_count();
	sub_sprite_info_array = (SubSpriteInfo*) mem_add( sizeof(SubSpriteInfo) * subSpriteCount );

	memset( sub_sprite_info_array, 0, sizeof(SubSpriteInfo) * subSpriteCount);

	for( int i=0 ; i<subSpriteCount; i++ )
	{
		SubSpriteRec *subSpriteRec = (SubSpriteRec *) dbSubSprite->read(i+1);
		SubSpriteInfo *subSpriteInfo = sub_sprite_info_array+i;

		subSpriteInfo->sprite_id = m.atoi(subSpriteRec->sub_sprite_id, subSpriteRec->RECNO_LEN);
		err_when( subSpriteInfo->sprite_id > sprite_info_count );
		subSpriteInfo->sprite_info = sprite_info_array + subSpriteInfo->sprite_id -1;
		subSpriteInfo->offset_x = m.atoi(subSpriteRec->offset_x, subSpriteRec->OFFSET_LEN);
		subSpriteInfo->offset_y = m.atoi(subSpriteRec->offset_y, subSpriteRec->OFFSET_LEN);

		// set link from parent
		// assume SUB_SPR database is sorted by sprite_name and sub_no

		int subNo = m.atoi(subSpriteRec->sub_no, subSpriteRec->SUB_NO_LEN);
		SpriteInfo *parentSprite = sprite_res[m.atoi(subSpriteRec->sprite_id, subSpriteRec->RECNO_LEN)];

		if( subNo == 1)
			parentSprite->sub_sprite_info = subSpriteInfo;

		parentSprite->sub_sprite_count = subNo;
	}
}
//-------- End of function SpriteRes::load_sub_sprite_info ---------//


//-------- Begin of function SpriteRes::update_speed -------//

void SpriteRes::update_speed()
{
	SpriteInfo *spriteInfo;
	short rainScale = weather.rain_scale();
	short snowScale = weather.snow_scale();
	short speedDrop;

	rainScale = rainScale > 7 ? 7 : rainScale;
	snowScale = snowScale > 7 ? 7 : snowScale;

	for( int i=0 ; i<sprite_info_count ; i++ )
	{
		speedDrop = 0;
		spriteInfo = sprite_info_array+i;

		if( rainScale > 0 && spriteInfo->max_rain_slowdown > 0 )
		{
			speedDrop += rainScale*spriteInfo->max_rain_slowdown/8 + 1;
		}

		if( snowScale > 0 && spriteInfo->max_snow_slowdown > 0 )
		{
			speedDrop += snowScale*spriteInfo->max_snow_slowdown/8 + 1;
		}
		spriteInfo->speed = spriteInfo->max_speed - speedDrop;
	}
}
//-------- End of function SpriteRes::update_speed -------//


#ifdef DEBUG

//-------- Begin of function SpriteRes::operator[] -------//

SpriteInfo* SpriteRes::operator[](int recNo)
{
	if( recNo<1 || recNo>sprite_info_count )
		err.run( "SpriteRes::operator[%d]", recNo );

	return sprite_info_array+recNo-1;
}

//--------- End of function SpriteRes::operator[] --------//

#endif

//------- Begin of function SpriteInfo::~SpriteInfo -------//

SpriteInfo::~SpriteInfo()
{
	res_bitmap.deinit();
	if(palw_ptr)
		mem_del(palw_ptr);
	palw_ptr = NULL;
}
//--------- End of function SpriteInfo::~SpriteInfo -------//


//------- Begin of function SpriteInfo::load_bitmap_res -------//

void SpriteInfo::load_bitmap_res()
{
	if( ++loaded_count > 1 )		// if bitmaps of this sprite has been loaded
		return;

	//----- open sprite bitmap resource file -------//

	String str;

	str  = DIR_SPRITE;
	str += sprite_code;
	str += ".SPR";

	res_bitmap.init_imported(str, 1);  // 1-read all into buffer

	//------ create color palette for eight nation ------//

	err_when( palw_ptr );		// should be cleared
	if( pal_ptr )
	{
		palw_ptr = (short *)mem_add( 0x100 * sizeof(short) );
		for( int j = 0; j < 0x100; ++j )
			palw_ptr[j] = vga.make_pixel( j+(RGBColor *)pal_ptr );
	}
}
//-------- End of function SpriteInfo::load_bitmap_res -------//


//------- Begin of function SpriteInfo::free_bitmap_res -------//

void SpriteInfo::free_bitmap_res()
{
	loaded_count--;

	err_when( loaded_count < 0 );

	if( loaded_count==0 )		// if this bitmap is still needed by other sprites
	{
		res_bitmap.deinit();

		if( palw_ptr )
			mem_del( palw_ptr );
		palw_ptr = NULL;
	}
}
//-------- End of function SpriteInfo::free_bitmap_res -------//


//------- Begin of function SpriteInfo::get_sub_sprite -------//
SpriteInfo *SpriteInfo::get_sub_sprite(int i)
{
	if( i < 1 || i > sub_sprite_count)
		return NULL;
	else
		return (sub_sprite_info+i-1)->sprite_info;
}
//------- End of function SpriteInfo::get_sub_sprite -------//


//------- Begin of function SpriteInfo::get_sub_sprite_info -------//
SubSpriteInfo *SpriteInfo::get_sub_sprite_info(int i)
{
	if( i < 1 || i > sub_sprite_count)
		return NULL;
	else
		return sub_sprite_info+i-1;
}
//------- End of function SpriteInfo::get_sub_sprite_info -------//


//-------- Begin of function SpriteInfo::travel_days ---------//
//
// <int> travelDistance - total distance in location units.
//
// return: <int> the no. of days it will take to travel the distance.
//
int SpriteInfo::travel_days(int travelDistance)
{
	int travelFrames = LOCATE_WIDTH * travelDistance / speed;

	return travelFrames / FRAMES_PER_DAY * 110 / 100;		// + 10% for circumstances that the units are blocked and needed to wait and turning, etc.
}
//--------- End of function SpriteInfo::travel_days ----------//


//--------- Begin of function SpriteInfo::get_color_remap_table --------//
//
// <int> nationRecno  - 0 for independent nation
// <int> selectedFlag - whether display outline around the bitmap
//
short* SpriteInfo::get_color_remap_table(int nationRecno, int selectedFlag)
{
	if( palw_ptr )
	{
		#define FIRST_REMAP_KEY       0xE0        // the source color code of the colors to be remapped
		#define REMAP_KEY_COUNT       8
		memcpy( palw_ptr+FIRST_REMAP_KEY,
			game.get_color_remap_table(nationRecno, selectedFlag) + FIRST_REMAP_KEY, (sizeof(short) *REMAP_KEY_COUNT) );

		return palw_ptr;
	}

	return game.get_color_remap_table(nationRecno, selectedFlag);
}
//--------- End of function SpriteInfo::get_color_remap_table --------//


// -------- begin of function SpriteRes::search_sprite --------//
//
// search certain sprite by name
// better to store the result in static variable 
//
int SpriteRes::search_sprite(const char *spriteName)
{
	for(int i = 0; i < sprite_info_count; ++i )
	{
		if( strcmp(spriteName, sprite_info_array[i].sprite_code) == 0 )
			return i+1;
	}

	return 0;
}
// -------- end of function SpriteRes::search_sprite --------//


void SpriteRes::fix_sprite_info()
{
	if( GAME_VERSION >= 119 )
	{
		// change turning of MO_SPU and PE_SPU
		int spriteRecno;

		if( (spriteRecno = search_sprite("MO_SPU")) != 0 )
			sprite_info_array[spriteRecno-1].need_turning = 2;
		if( (spriteRecno = search_sprite("PE_SPU")) != 0 )
			sprite_info_array[spriteRecno-1].need_turning = 2;
	}
}

