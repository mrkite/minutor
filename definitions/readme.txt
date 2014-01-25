To allow testing for mob spawn conditions some flags have to be set according
to the Minecraft source code. The Minecraft Wiki is not correct in all cases.

In the Minecraft source code some major flags are defined for all blocks:
 isOpaque renderAsNormalBlock isLiquid and canProvidePower

isLiquid:
 is obvious and true for still and flowing water and lava

canProvidePower:
 is obvious all blocks that could create a redstone signal:
  PressurePlate(s), Button(s), TrappedChest, DaylightDetector, DetectorRail,
  Lever, PoweredOre = Redstone Block, RedstoneLogic = Repeater and Comperator,
  RedstoneTorch, RestoneWire, TripWireSource = Tripwire Hook

isOpaque and renderAsNormalBlock
 both are more or less the same except some rare special cases:
  opaque blocks are always rendered as normal blocks
  in case they are not opaque (= transparent) they are mostely
  rendered NOT as normal block except:
   Ice, EndPortalFrame, Leaves (all kind), MobSpawner,
   (also: Redstone Wire in case not powered)

  to detect that special case, this tag has to be placed
      "transparent": true,
      "rendercube": true

  all other transparent blocks have to be flagged only with
      "transparent": true

List of tranparent blocks:
 Anvil, PressurePlate(s), Beacon, Bed, Ice, BrewingStand, Button(s), Cactus,
 Cake, Carpet(s), Cauldron, Chest(s), Cocoa, DaylightDetector, Door(s), DragonEgg,
 EnchantmentTable, EnderChest, EndPortal, EndPortalFrame, Farmland, Fence,
 FenceGate, Fire, Flower(s), FlowerPot, Fluid = Water/Lava, Glass, HalfSlab(s),
 Hopper, Ladder, Leave(s), Lever, MobSpawner, Pane(s), PistonBase, PistonExtension,
 PistonMoving, Portal, Rail(s), Repeater, Comperator, RedstoneWire, Reed, Sign,
 Skull, Snow, Stair(s), Torch, TrapDoor, TripWire, TripWireSource, Vine, Wall, Web

All this information is gathered from the 1.6.4 source code.
As at the moment no deobfuscated 1.7.x code is available.

[EtlamGit]