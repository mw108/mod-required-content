/**
 * Copyright (c) 2024 Mario Ewert - mario@108bits.de
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"

using namespace std;

bool requiredContentEnabled;
std::set<std::pair<uint8, std::set<uint32>>> requiredLevelAchievements;

class RequiredContentConfig : public WorldScript
{
    public:
        bool isEnabled;

        RequiredContentConfig() : WorldScript( "RequiredContentConf" ) { }

        std::set<uint32> getCommaSeparatedIntegers( std::string text )
        {
            std::string         value;
            std::stringstream   stream;
            std::set<uint32>    result;

            stream.str( text );

            while ( std::getline( stream, value, ','))
            {
                result.insert( atoi( value.c_str() ) );
            }

            return result;
        }

        void OnBeforeConfigLoad(bool /*reload*/) override
        {
            LOG_INFO( "module", "Loading Required Content Config" );
            requiredContentEnabled = sConfigMgr->GetOption<bool>( "RequiredContent.enabled", false, false );
            for ( uint8 level = 1; level <= 80; level++ )
            {
                std::string levelAchievements = sConfigMgr->GetOption<std::string>( "RequiredContent.Level." + std::to_string( level ), "", false );
                if ( levelAchievements.length() > 0 )
                {
                    std::set<uint32> achievementIds = getCommaSeparatedIntegers( levelAchievements );
                    requiredLevelAchievements.insert( std::make_pair( level, achievementIds ) );
                    LOG_INFO( "module", "Achievement(s) for level {} required: {}", level, levelAchievements  );
                }
            }
        }
};

class RequiredContent : public PlayerScript
{
    public:
        RequiredContent() : PlayerScript( "RequiredContent" ) { }

        void OnPlayerGiveXP( Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/ ) override
        {
            if ( ! requiredContentEnabled || player->GetSession()->IsBot() )
            {
                return;
            }

            std::set<std::pair<uint8, std::set<uint32>>>::iterator it;
            uint8 playerLevel = player->GetLevel();
            for ( it = requiredLevelAchievements.begin(); it != requiredLevelAchievements.end(); ++it )
            {
                uint8 requiredLevel = it->first;
                if ( playerLevel >= requiredLevel )
                {
                    std::set<uint32> requiredAchievements = it->second;
                    std::set<uint32>::iterator subIt;
                    for ( subIt = requiredAchievements.begin(); subIt != requiredAchievements.end(); ++subIt )
                    {
                        uint32 achievementId = *subIt;
                        bool hasAchieved = player->HasAchieved( achievementId );
                        if ( ! hasAchieved )
                        {
                            amount = 0;
                            return;
                        }
                    }
                }
            }
        }

};

void AddRequiredContentScripts()
{
    new RequiredContentConfig();
    new RequiredContent();
}
