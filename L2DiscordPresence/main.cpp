//  Copyright (C) 2022 Herald Gersdorff <heraldgersdorff@yandex.com>.
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "pch/pch.h"

constexpr auto APP_ID = 0LL; // set your application id here (from https://discord.com/developers/applications/ )

struct Application
{
    IDiscordCore*            core;
    IDiscordUserManager*     users;
    IDiscordActivityManager* activities;
}                            g_App;

// import this function to your appliaction's .exe or .dll (e.g. ogg.dll for Lineage 2)
__declspec(dllexport) int APIENTRY Anchor();

void initLib();

DWORD APIENTRY LibEntry( LPVOID );

void initDiscord();
void updateActivity( const Application& app );

bool APIENTRY DllMain( const HINSTANCE hinstDLL
                     , const DWORD     fdwReason
                     , LPVOID )
{
    switch ( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( hinstDLL );
        initLib();

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    default:
        break;
    }

    return true;
}

void initLib()
{
    CreateThread( nullptr, 0, LibEntry, nullptr, 0, nullptr );
}

DWORD APIENTRY LibEntry( LPVOID )
{
    initDiscord();
    return 0;
}

void initDiscord()
{
    g_App = {};
    DiscordCreateParams params = {};

    DiscordCreateParamsSetDefault( &params );
    params.client_id = APP_ID;
    params.flags = DiscordCreateFlags_NoRequireDiscord; // dont force discord to launch
    params.event_data = &g_App;

    // disable integration in case of any error (otherwise your appliction may crash)
    if ( DiscordCreate( DISCORD_VERSION, &params, &g_App.core ) != DiscordResult_Ok )
    {
        return;
    }

    g_App.users = g_App.core->get_user_manager( g_App.core );
    g_App.activities = g_App.core->get_activity_manager( g_App.core );

    updateActivity( g_App );

    while ( true )
    {
        if ( g_App.core->run_callbacks( g_App.core ) != DiscordResult_Ok )
        {
            return;
        }

        Sleep( 16 );
    }
}

void updateActivity( const Application& app )
{
    DiscordActivity activity{};
    activity.type = DiscordActivityType_Playing;
    activity.application_id = APP_ID;

    // see preview located in repo/assets/preview.png
    sprintf( activity.details, "very details" );
    sprintf( activity.state, "such state" );

    // Assets section (e.g. icons and text on-hover for them)
    DiscordActivityAssets assets{};
    sprintf( assets.large_image, "test_big_pic" );
    sprintf( assets.large_text, "big text" );
    sprintf( assets.small_image, "test_smol_pic" );
    sprintf( assets.small_text, "smol text" );

    activity.assets = assets;
    // Assets section - END

    // Timestamps section
    DiscordActivityTimestamps timestamps{};

    // '<time> elapsed' label
    timestamps.start = std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() ).count();

    activity.timestamps = timestamps;
    // Timestamps section - END

    app.activities->update_activity( app.activities, &activity, nullptr, nullptr );
}

int APIENTRY Anchor()
{
    return 0;
}
