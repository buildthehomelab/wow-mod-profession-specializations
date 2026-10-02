/*
 * mod-all-specializations
 *
 * Teaches every specialization of a profession (Gnomish + Goblin Engineering, Armorsmith +
 * Weaponsmith and the three weapon masteries, all Leatherworking, Tailoring and Alchemy specs)
 * once the player meets the skill and level the stock specialization quests ask for.
 *
 * Specializations gate three things: trainer recipes (trainer_spell ReqAbility), recipe items and
 * the crafted gear itself (item_template requiredspell). Having the spell satisfies all three, so
 * nothing else needs changing.
 *
 * Released under the MIT License.
 */

#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <string>
#include <vector>

namespace
{
    struct Specialization
    {
        uint32 skill;
        uint16 minSkill;
        uint8 minLevel;
        uint32 requiredSpell; // also needed first (weapon masteries need Weaponsmith); 0 = none
        uint32 spell;
    };

    // Skill and level follow the stock specialization quests (and npc_professions for the weapon
    // masteries, which have no quest).
    std::vector<Specialization> const Specializations =
    {
        { SKILL_ENGINEERING,    200, 10, 0,    20219 }, // Gnomish Engineer
        { SKILL_ENGINEERING,    200, 10, 0,    20222 }, // Goblin Engineer
        { SKILL_BLACKSMITHING,  200, 40, 0,    9788  }, // Armorsmith
        { SKILL_BLACKSMITHING,  200, 40, 0,    9787  }, // Weaponsmith
        { SKILL_BLACKSMITHING,  250, 50, 9787, 17039 }, // Master Swordsmith
        { SKILL_BLACKSMITHING,  250, 50, 9787, 17040 }, // Master Hammersmith
        { SKILL_BLACKSMITHING,  250, 50, 9787, 17041 }, // Master Axesmith
        { SKILL_LEATHERWORKING, 225, 40, 0,    10656 }, // Dragonscale Leatherworking
        { SKILL_LEATHERWORKING, 225, 40, 0,    10658 }, // Elemental Leatherworking
        { SKILL_LEATHERWORKING, 225, 40, 0,    10660 }, // Tribal Leatherworking
        { SKILL_TAILORING,      325, 60, 0,    26797 }, // Spellfire Tailoring
        { SKILL_TAILORING,      325, 60, 0,    26798 }, // Mooncloth Tailoring
        { SKILL_TAILORING,      325, 60, 0,    26801 }, // Shadoweave Tailoring
        { SKILL_ALCHEMY,        325, 68, 0,    28672 }, // Transmutation Master
        { SKILL_ALCHEMY,        325, 68, 0,    28675 }, // Potion Master
        { SKILL_ALCHEMY,        325, 68, 0,    28677 }, // Elixir Master
    };

    struct Config
    {
        bool enabled = true;
        bool announce = true;
    };

    Config config;

    bool IsSpecializationSkill(uint32 skill)
    {
        for (Specialization const& spec : Specializations)
            if (spec.skill == skill)
                return true;

        return false;
    }

    void GrantSpecializations(Player* player)
    {
        if (!config.enabled || !player)
            return;

        std::vector<uint32> learned;

        // In table order, so Weaponsmith is learned before the masteries that require it.
        for (Specialization const& spec : Specializations)
        {
            if (player->HasSpell(spec.spell))
                continue;
            if (!player->HasSkill(spec.skill) || player->GetBaseSkillValue(spec.skill) < spec.minSkill)
                continue;
            if (player->GetLevel() < spec.minLevel)
                continue;
            if (spec.requiredSpell && !player->HasSpell(spec.requiredSpell))
                continue;
            if (!sSpellMgr->GetSpellInfo(spec.spell))
                continue;

            player->learnSpell(spec.spell);
            learned.push_back(spec.spell);
        }

        if (learned.empty() || !config.announce)
            return;

        LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
        std::string names;
        for (uint32 spellId : learned)
        {
            if (!names.empty())
                names += ", ";
            names += sSpellMgr->GetSpellInfo(spellId)->SpellName[locale];
        }

        ChatHandler(player->GetSession()).PSendSysMessage("New specialization{}: {}.",
            learned.size() == 1 ? "" : "s", names);
    }
}

class AllSpecializationsWorldScript : public WorldScript
{
public:
    AllSpecializationsWorldScript() : WorldScript("AllSpecializationsWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        config.enabled  = sConfigMgr->GetOption<bool>("AllSpecializations.Enable", true);
        config.announce = sConfigMgr->GetOption<bool>("AllSpecializations.Announce", true);
    }
};

class AllSpecializationsPlayerScript : public PlayerScript
{
public:
    AllSpecializationsPlayerScript() : PlayerScript("AllSpecializationsPlayerScript",
        { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LEVEL_CHANGED, PLAYERHOOK_ON_UPDATE_SKILL, PLAYERHOOK_ON_SET_SKILL }) { }

    // Existing characters that already qualify, and anyone who unlearned one at a trainer.
    void OnPlayerLogin(Player* player) override
    {
        GrantSpecializations(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        GrantSpecializations(player);
    }

    // Skill-ups from crafting and gathering.
    void OnPlayerUpdateSkill(Player* player, uint32 skillId, uint32 /*value*/, uint32 /*max*/, uint32 /*step*/, uint32 /*newValue*/) override
    {
        if (IsSpecializationSkill(skillId))
            GrantSpecializations(player);
    }

    // Skill set directly: training a rank, GM commands, quests.
    void OnPlayerSetSkill(Player* player, uint32 skillId, uint32 /*value*/, uint32 /*max*/, uint32 /*step*/, uint32 /*newValue*/) override
    {
        if (IsSpecializationSkill(skillId))
            GrantSpecializations(player);
    }
};

void AddAllSpecializationsScripts()
{
    new AllSpecializationsWorldScript();
    new AllSpecializationsPlayerScript();
}
