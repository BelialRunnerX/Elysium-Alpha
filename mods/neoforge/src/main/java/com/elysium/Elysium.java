package com.elysium;

import com.elysium.affix.ElysiumAffixes;
import com.elysium.block.NeutroniumBlock;
import com.elysium.block.NeutroniumOreBlock;
import com.elysium.item.ElysiumArmorItem;
import com.elysium.item.ElysiumArmorMaterials;
import com.elysium.item.ElysiumReforgeItem;
import com.elysium.item.ElysiumRuneItem;
import com.elysium.silentgear.ElysiumSilentGear;
import com.elysium.tooltip.ElysiumLegendaryTooltips;
import net.minecraft.world.item.BlockItem;
import net.minecraft.world.item.CreativeModeTabs;
import net.minecraft.world.item.Item;
import net.minecraftforge.common.MinecraftForge;
import net.minecraftforge.event.BuildCreativeModeTabContentsEvent;
import net.minecraftforge.eventbus.api.IEventBus;
import net.minecraftforge.fml.common.Mod;
import net.minecraftforge.fml.javafmlmod.FMLJavaModLoadingContext;
import net.minecraftforge.registries.DeferredRegister;
import net.minecraftforge.registries.ForgeRegistries;
import net.minecraftforge.registries.RegistryObject;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

@Mod(Elysium.MODID)
public class Elysium {
    public static final String MODID = "elysium";
    public static final Logger LOGGER = LogManager.getLogger();

    public static final DeferredRegister<Item> ITEMS = 
        DeferredRegister.create(ForgeRegistries.ITEMS, MODID);

    public static final DeferredRegister<net.minecraft.world.level.block.Block> BLOCKS = 
        DeferredRegister.create(ForgeRegistries.BLOCKS, MODID);

    // Blocks
    public static final RegistryObject<net.minecraft.world.level.block.Block> NEUTRONIUM_ORE = BLOCKS.register("neutronium_ore",
        NeutroniumOreBlock::new);

    public static final RegistryObject<net.minecraft.world.level.block.Block> NEUTRONIUM_BLOCK = BLOCKS.register("neutronium_block",
        NeutroniumBlock::new);

    public static final RegistryObject<Item> NEUTRONIUM_ORE_ITEM = ITEMS.register("neutronium_ore",
        () -> new BlockItem(NEUTRONIUM_ORE.get(), new Item.Properties()));

    public static final RegistryObject<Item> NEUTRONIUM_BLOCK_ITEM = ITEMS.register("neutronium_block",
        () -> new BlockItem(NEUTRONIUM_BLOCK.get(), new Item.Properties()));

    public static final RegistryObject<Item> NEUTRONIUM_INGOT = ITEMS.register("neutronium_ingot",
        () -> new Item(new Item.Properties()));

    // Armor
    public static final RegistryObject<Item> ELYSIUM_HELMET = ITEMS.register("elysium_helmet",
        () -> new ElysiumArmorItem(ElysiumArmorMaterials.ELYSIUM, 
            net.minecraft.world.item.ArmorItem.Type.HELMET, 
            new Item.Properties(), 0, 2));

    public static final RegistryObject<Item> PLASMA_CHESTPLATE = ITEMS.register("plasma_chestplate",
        () -> new ElysiumArmorItem(ElysiumArmorMaterials.ELYSIUM, 
            net.minecraft.world.item.ArmorItem.Type.CHESTPLATE, 
            new Item.Properties(), 1, 3));

    public static final RegistryObject<Item> NEURAL_LEGGINGS = ITEMS.register("neural_leggings",
        () -> new ElysiumArmorItem(ElysiumArmorMaterials.ELYSIUM, 
            net.minecraft.world.item.ArmorItem.Type.LEGGINGS, 
            new Item.Properties(), 2, 2));

    public static final RegistryObject<Item> DIMENSIONAL_BOOTS = ITEMS.register("dimensional_boots",
        () -> new ElysiumArmorItem(ElysiumArmorMaterials.ELYSIUM, 
            net.minecraft.world.item.ArmorItem.Type.BOOTS, 
            new Item.Properties(), 3, 3));

    // Runes
    public static final RegistryObject<Item> VOIDWARD_RUNE = ITEMS.register("voidward_rune",
        () -> new ElysiumRuneItem(ElysiumRuneItem.RuneType.VOIDWARD, new Item.Properties()));

    public static final RegistryObject<Item> PLASMAFORGE_RUNE = ITEMS.register("plasmaforge_rune",
        () -> new ElysiumRuneItem(ElysiumRuneItem.RuneType.PLASMAFORGE, new Item.Properties()));

    public static final RegistryObject<Item> NEURALSPIKE_RUNE = ITEMS.register("neuralspike_rune",
        () -> new ElysiumRuneItem(ElysiumRuneItem.RuneType.NEURALSPIKE, new Item.Properties()));

    public static final RegistryObject<Item> DIMENSIONALSHIFT_RUNE = ITEMS.register("dimensionalshift_rune",
        () -> new ElysiumRuneItem(ElysiumRuneItem.RuneType.DIMENSIONALSHIFT, new Item.Properties()));

    public static final RegistryObject<Item> KINETICSURGE_RUNE = ITEMS.register("kineticsurge_rune",
        () -> new ElysiumRuneItem(ElysiumRuneItem.RuneType.KINETICSURGE, new Item.Properties()));

    // Reforge Item
    public static final RegistryObject<Item> ELYSIUM_REFORGE = ITEMS.register("elysium_reforge",
        () -> new ElysiumReforgeItem(new Item.Properties()));

    // Example Unique item
    public static final RegistryObject<Item> EMPEROR_CROWN = ITEMS.register("emperor_crown",
        () -> new ElysiumArmorItem(ElysiumArmorMaterials.ELYSIUM, 
            net.minecraft.world.item.ArmorItem.Type.HELMET, 
            new Item.Properties(), 0, 5));

    public Elysium() {
        IEventBus modEventBus = FMLJavaModLoadingContext.get().getModEventBus();
        
        ITEMS.register(modEventBus);
        BLOCKS.register(modEventBus);
        
        MinecraftForge.EVENT_BUS.register(this);
        modEventBus.addListener(this::addCreative);
        
        // Apotheosis integration (soft dependency)
        if (net.minecraftforge.fml.ModList.get().isLoaded("apotheosis")) {
            ElysiumAffixes.register(modEventBus);
            LOGGER.info("Apotheosis detected - enabling affix integration");
        }
        
        // Silent Gear integration (soft dependency)
        if (net.minecraftforge.fml.ModList.get().isLoaded("silentgear")) {
            ElysiumSilentGear.register();
        }
        
        // Legendary Tooltips integration (soft dependency)
        if (net.minecraftforge.fml.ModList.get().isLoaded("legendarytooltips")) {
            ElysiumLegendaryTooltips.register();
        }
        
        LOGGER.info("Elysium mod initialized");
    }

    private void addCreative(BuildCreativeModeTabContentsEvent event) {
        if (event.getTabKey() == CreativeModeTabs.BUILDING_BLOCKS) {
            event.accept(NEUTRONIUM_BLOCK_ITEM);
        }
        if (event.getTabKey() == CreativeModeTabs.NATURAL_BLOCKS) {
            event.accept(NEUTRONIUM_ORE_ITEM);
        }
        if (event.getTabKey() == CreativeModeTabs.INGREDIENTS) {
            event.accept(NEUTRONIUM_INGOT);
            event.accept(ELYSIUM_REFORGE);
        }
        if (event.getTabKey() == CreativeModeTabs.COMBAT) {
            event.accept(ELYSIUM_HELMET);
            event.accept(PLASMA_CHESTPLATE);
            event.accept(NEURAL_LEGGINGS);
            event.accept(DIMENSIONAL_BOOTS);
            event.accept(EMPEROR_CROWN);
        }
        if (event.getTabKey() == CreativeModeTabs.TOOLS_AND_UTILITIES) {
            event.accept(VOIDWARD_RUNE);
            event.accept(PLASMAFORGE_RUNE);
            event.accept(NEURALSPIKE_RUNE);
            event.accept(DIMENSIONALSHIFT_RUNE);
            event.accept(KINETICSURGE_RUNE);
        }
    }
}