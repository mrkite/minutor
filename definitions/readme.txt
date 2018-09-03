To allow testing for mob spawn conditions some flags have to be set according
to the Minecraft source code. The Minecraft Wiki is not correct in all cases.

In the Minecraft source code some major flags are defined for all blocks:
 isOpaque renderAsNormalBlock isLiquid and canProvidePower

Additionally mobs can spawn inside/through some transparent blocks which is
checked with hit box collision. As this is not possible to check in Minutor
a special flag "spawninside" has to be set for that blocks.


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

  all other transparent blocks have to be tagged only with
      "transparent": true

List of tranparent blocks:
 Anvil, PressurePlate(s), Beacon, Bed, Ice, BrewingStand, Button(s), Cactus,
 Cake, Carpet(s), Cauldron, Chest(s), Cocoa, DaylightDetector, Door(s),
 DragonEgg, EnchantmentTable, EnderChest, EndPortal, EndPortalFrame, Farmland,
 Fence, FenceGate, Fire, Flower(s), FlowerPot, Fluid = Water/Lava, Glass,
 HalfSlab(s), Hopper, Ladder, Leave(s), Lever, MobSpawner, Pane(s), PistonBase,
 PistonExtension, PistonMoving, Portal, Rail(s), Repeater, Comperator,
 RedstoneWire, Reed, Sign, Skull, Snow, Stair(s), Torch, TrapDoor, TripWire,
 TripWireSource, Vine, Wall, Web


List of blocks that mobs can spawn inside:
 PressurePlate(s), Button(s), Cocoa, (single block) Flower(s), Ladder, Lever,
 Rail(s), Reed, Sign, Snow, Vine
to be flagged with
  "spawninside": true


All this information is gathered directly from the 1.6.4 - 1.7.10 source code
or special test setup per block in development snapshots. As at the moment
deobfuscated 1.8.x code is not available.



Other attribute value pairs in this file:

"id": integer_number

Numerical code used in the savegame for this block


"name": "Name of Block"

String representation of this Block, shown in status bar when hovering mouse over


"color": "#ffffff"

Color used for rendering this block
 For new Blocks, please use the average color of the texture Minecraft would use for the top side.
 ImageMagick can do that for you:
 > convert ${filename} -filter box -resize 1x1 -depth 8 txt:-


"alpha": float_number

In case the block is transparent (e.g. water) this defines the amount it coveres
the block below. 0.0 is completely transparent. 1.0 is the default, a opaque block.


"mask": integer_number

In case a block has variants a mask can be given to blend out bits in the data
field that are not used to define the block itself. These bits are mostely used
to define block orientation.
 if a block has no variants defined,   "mask":  0 is predefined
 if a block has some variants defined, "mask": 15 is predefined


"biomeGrass": boolean

When color of block is changing based on Biome and is based on the Grass Colorizer,
this flag has to be set true. Defaults to false.


"biomeFoliage": boolean

When color of block is changing based on Biome and is based on the Foliage Colorizer,
this flag has to be set true. Defaults to false.


[EtlamGit]