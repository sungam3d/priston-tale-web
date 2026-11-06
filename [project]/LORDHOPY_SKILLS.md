# Adding Lord Hopy's skills to the game source

How to put Lord Hopy's skills, animations and HP phases into a copy of the Priston Tale source. Use this for another
server or a fresh checkout. The monster must already load and spawn; this guide adds what he does in a fight.

**You will change** six source files, add 11 particle scripts and a sound folder, replace his model files, and rebuild
both the client and the server.

## What you get

| HP | Skill | Key | Animation (frames, hit) | Effect | Damage |
|---|---|---|---|---|---|
| Any | Golpe Sísmico (his normal attack) | none | attack, 230-290, hit 264 | `LordHopySlam` | Normal attack |
| Any | Aliento de Energía | `B` | roar, 300-390, hit 352 | `LordHopyMouth`, `LordHopyBreath`, `LordHopyBreathHit` | Target only |
| 70% and below | Invocación de Cristales | `C` | crystal, 500-590, hit 552 | `LordHopyMark`, `LordHopyCrystal` | Area, radius 170 |
| 70% and below | Ilusiones de Hopy | `I` | illusion, 600-720, hit 650 | `LordHopyIllusion`, plus 3 drawn copies | None |
| 30% and below | Éxtasis Arcano | `E` | ecstasy, 730-860, hit 812 | `LordHopyCharge`, `LordHopyBlast` | Area, radius 340 |

HP phases add a pulsing violet tint and an aura: `LordHopyAura2` from 70% down, `LordHopyAura3` from 30% down.

## Files you need

| What | Where in this repo |
|---|---|
| Source patch | `[project]/lordhopy_skills.patch` |
| Model | `data/char/monster/LordHopy/LordHopy.{bmp,inx,smb,smd}` |
| Particle scripts | `data/Effect/Particle/Script/LordHopy*.part` |
| Sounds | `data/wav/Effects/monster/LordHopy/` (a copy of Hopy King's for now) |
| Server definition | `[project]/dist/server/GameServer/Monster/B_LordHopy.inf` |

## Step 1: apply the source patch

Run this from the source root, the folder that holds `character.cpp`:

```bash
git apply --whitespace=nowarn ../lordhopy_skills.patch
```

If your source differs and the patch fails, make the changes by hand using Step 2. Search by symbol name, because line
numbers will not match.

## Step 2: what the patch changes

### Give him his own effect code

Monster effects are chosen by `dwCharSoundCode`, so a unique code keeps Hopy King and his variants unchanged.

| File | Change |
|---|---|
| `effectsnd.h` | `#define snCHAR_SOUND_LORDHOPY 0x2190`. Check that the value is unused. |
| `effectsnd.cpp` | Add `{ "wav\\Effects\\monster\\LordHopy\\", snCHAR_SOUND_LORDHOPY },` to `snFindEffects`, and **raise `snFIND_EFFECT_MAX` by one** or it will not compile. |
| `fileread.cpp` | Add `{ "LORDHOPY", snCHAR_SOUND_LORDHOPY },` to the `dwCharSoundCode` table, above its terminator. This is what links `*효과음 LORDHOPY` in the `.inf` to the code. |

### Load the particle scripts

| File | Change |
|---|---|
| `HoBaram/HoNewParticleMgr.h` | Raise `MAX_TOKEN_VECTORS` from 250 to 512. The game already asks for 251 scripts, and any past the limit are **dropped with no error**. |
| `HoBaram/HoEffect.cpp` | At the end of the `LoadScript` list in `InitEffect()`, add one `g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyX.part");` line per script. A script's name is its file name. |

### Add the skill logic in `character.cpp`

1. **Include** `hobaram\\HoNewParticleLinkHeader.h`.
2. **Add helpers** before `GetDistanceDbl`:
   - `LordHopyFx`: starts a script. Its body is inside `#ifndef PT_SERVER_EXE`, because this file also builds into the server.
   - `LordHopyPhase`: returns 1, 2 or 3 from `Life[0]` and `Life[1]`.
   - `LordHopyPickSkill`: redraws the random skill until the HP phase allows it.
   - `LordHopyCrystalPos`: gives the fixed crystal ring, so the warning marks and the needles line up.
3. **`EventAttack`:** add a `case snCHAR_SOUND_LORDHOPY` that starts `LordHopySlam` in front of him.
4. **Skill choice:** call `LordHopyPickSkill( this )` straight after both `SetMotionFromCode( CHRMOTION_STATE_SKILL )`
   calls in the monster attack code.
5. **Skill damage**, where the server tests `FrameCounter==EvtFrame`:
   - For him, test the motion's own hit frame instead. Otherwise the fixed timing lands long before Éxtasis visibly goes off.
   - `I` deals no damage.
   - `B` sets `AttackSkillRange = 0`, which sends the damage to his target only.
   - `E` multiplies `AttackSkillRange` by 4. It is a squared value, so the radius doubles.
6. **HP aura:** in the per-tick code beside the `MONSTER_CLASS_BOSS` death check, start `LordHopyAura2` or `LordHopyAura3`
   when `(Counter&0x7F)==0`, if he is drawn and alive.
7. **HP tint and illusions**, in the character draw function:
   - After `smRender.Color_A` is set, add the pulsing violet to `Color_R` and `Color_B`.
   - After the main pattern is rendered, for motion `I`, draw him 3 more times at offset positions with a lowered
     `Color_A`, then restore the position and colours.
8. **Bone lookup:** in the setup switch that assigns `AttackObjBip`, add his case with `GetObjectFromName( "Bip05" )`.
   `GetAttackPoint` then returns his snout, where the beam starts.
9. **`BeginSkill_Monster` and `EventSkill_Monster`:** add a `case snCHAR_SOUND_LORDHOPY` that switches on
   `MotionInfo->KeyCode` and starts the effects at the start of the motion and on its hit frame.

## Step 3: install the data

**Client data folder:**
- Copy the four model files to `char/monster/LordHopy/`.
- Copy the 11 `.part` files to `Effect/Particle/Script/`.
- Copy the sound folder to `wav/Effects/monster/LordHopy/`.

**Server folder:**
- Copy `LordHopy.smd`, `.smb`, `.inx` and `.bmp` to `char/monster/LordHopy/`.
- Copy `B_LordHopy.inf` to `GameServer/Monster/`.

**Browser build only:** every new or changed client file needs its line in `data_manifest.txt`, in the form
`path<TAB>size<TAB>hash`. The hash is `hashlib.blake2b(data, digest_size=8).hexdigest()`. Add only his lines by hand or
with a script: `web/make_manifest.py` lists everything in the folder, including stray copies.

## Step 4: rebuild and restart

Rebuild **both**. The client and the server share these tables.

```bash
python web/build_web.py --release
```

```bash
MSBuild PristonServer.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:PtGfx=GL /p:SolutionDir=<source folder>\
```

Then:
1. Stop **every** `server.exe`. If an autostart task exists, disable it first so it cannot restart the old exe mid-swap.
2. Replace `server.exe`, and check that the file really changed. A running copy locks it, so the copy can fail.
3. Start one instance, and re-enable the task.

## Step 5: test

Summon him as a GM with `/monster "Lord Hopy"`.

| HP | You should see |
|---|---|
| Any | A ground ring and dust on normal attacks; the breath beam. Nothing else should appear above 70%. |
| 70% and below | Warning marks then crystals; 3 purple copies; a violet pulse and drifting motes |
| 30% and below | The Éxtasis circle then blast; a stronger aura and lightning |

## If something goes wrong

| Symptom | Cause | Fix |
|---|---|---|
| Animations play but there are no effects, tint or phases | The server is still the old build, so `LORDHOPY` is not recognised and his code becomes 0 | Replace `server.exe` and confirm the file changed |
| Dropped from the game when he appears | The client cannot load his model, usually a missing manifest line | Add his files to `data_manifest.txt` |
| Effect missing, no error | Script not loaded: limit reached, or no manifest line | Check `MAX_TOKEN_VECTORS` and the manifest |
| Dark squares around an effect | A `.bmp` such as `Dust1.bmp` has no alpha. `BLEND_ALPHA` draws its black corners, and `BLEND_COLOR` still showed squares in the browser build | Use `BLEND_LAMP`, which only adds light, so black corners cannot show |
| Script cut short | The engine reads scripts into a 6000-byte buffer | Keep each `.part` under 6000 bytes |
| `.inx` will not load | Its data is signed with a hash of its own path | Keep it at `char\monster\LordHopy\LordHopy.inx`, or regenerate it |
| Client rebuilds or quits | `LordHopy.ASE` or `.ini` sits beside the compiled files | Keep source files out of that folder |

## Tuning

| To change | Edit |
|---|---|
| Level, HP, attack, skill damage, skill chance | `B_LordHopy.inf` (`*기술공격률`, `*기술공격력`). Current values are placeholders. Server restart needed. |
| Crystal damage radius | `*기술공격범위` in the `.inf` (Éxtasis is always double). Keep it larger than the ring in `LordHopyCrystalPos`. |
| HP thresholds | `LordHopyPhase` in `character.cpp`. Rebuild both. |
| Effect look | The `.part` scripts. Update the manifest; no rebuild needed. |
| New spawn point | Add him to a map's `GameServer/Field/*.spm`. He currently appears only when summoned. |
