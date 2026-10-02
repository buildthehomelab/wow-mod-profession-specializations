# mod-all-specializations

AzerothCore module: players learn **every** specialization of a profession instead of choosing one.

| Profession | Specializations | Skill | Level |
|---|---|---|---|
| Engineering | Gnomish, Goblin | 200 | 10 |
| Blacksmithing | Armorsmith, Weaponsmith | 200 | 40 |
| Blacksmithing | Master Swordsmith, Hammersmith, Axesmith (after Weaponsmith) | 250 | 50 |
| Leatherworking | Dragonscale, Elemental, Tribal | 225 | 40 |
| Tailoring | Spellfire, Mooncloth, Shadoweave | 325 | 60 |
| Alchemy | Transmutation, Potion, Elixir Master | 325 | 68 |

The skill and level match the stock specialization quests (the weapon masteries have no quest; they match the trainer gossip in core `npc_professions.cpp`).

## How it works

A specialization gates three things:

- trainer recipes (`trainer_spell.ReqAbility`)
- recipe items (`item_template.requiredspell`)
- the crafted gear and gadgets themselves (`item_template.requiredspell`, e.g. Gnomish goggles, Goblin Rocket Launcher, Spellfire set)

Knowing the specialization spell satisfies all three, so the module only teaches the spells. It checks on login, on level-up and whenever a profession skill changes. It needs no SQL and no client patch.

Existing characters that already qualify get their specializations the next time they log in.

The stock quests and trainer gossip still work. Unlearning a specialization at a trainer is pointless: it comes back at the next login or skill-up, but the recipes the trainer removed have to be trained again.

On a mod-individual-progression server, the 325-skill specializations (Tailoring, Alchemy) wait for the TBC tier, because the skill cap stops players reaching 325 before then.

## Configuration

`conf/mod_all_specializations.conf.dist`:

| Key | Default | Meaning |
|-----|---------|---------|
| `AllSpecializations.Enable` | 1 | Master switch |
| `AllSpecializations.Announce` | 1 | Chat message listing newly learned specializations |

## Install

```bash
cd modules
git clone https://github.com/buildthehomelab/wow-mod-all-specializations.git mod-all-specializations
# CMake reconfigure + rebuild
```

Clone into `mod-all-specializations`: AzerothCore derives the loader symbol from the folder name.

## License

MIT (see [LICENSE](LICENSE)).
