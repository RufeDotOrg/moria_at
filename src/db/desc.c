
static char* colors[] = {
    /* Do not move the first three */
    "Icky Green",
    "Light Brown",
    "Clear",
    "Azure",
    "Blue",
    "Blue Speckled",
    "Black",
    "Brown",
    "Brown Speckled",
    "Bubbling",
    "Chartreuse",
    "Cloudy",
    "Copper Speckled",
    "Crimson",
    "Cyan",
    "Dark Blue",
    "Dark Green",
    "Dark Red",
    "Gold Speckled",
    "Green",
    "Green Speckled",
    "Grey",
    "Grey Speckled",
    "Hazy",
    "Indigo",
    "Light Blue",
    "Light Green",
    "Magenta",
    "Metallic Blue",
    "Metallic Red",
    "Metallic Green",
    "Metallic Purple",
    "Misty",
    "Orange",
    "Orange Speckled",
    "Pink",
    "Pink Speckled",
    "Puce",
    "Purple",
    "Purple Speckled",
    "Red",
    "Red Speckled",
    "Silver Speckled",
    "Smoky",
    "Tangerine",
    "Violet",
    "Vermilion",
    "White",
    "Yellow",
};

static char* mushrooms[] = {
    "Blue",       "Black",      "Black Spotted", "Brown",         "Dark Blue",
    "Dark Green", "Dark Red",   "Ecru",          "Furry",         "Green",
    "Grey",       "Light Blue", "Light Green",   "Plaid",         "Red",
    "Slimy",      "Tan",        "White",         "White Spotted", "Wooden",
    "Wrinkled",   "Yellow",
};

static char* woods[] = {
    "Aspen",      "Balsa",    "Banyan",   "Birch",  "Cedar",
    "Cottonwood", "Cypress",  "Dogwood",  "Elm",    "Eucalyptus",
    "Hemlock",    "Hickory",  "Ironwood", "Locust", "Mahogany",
    "Maple",      "Mulberry", "Oak",      "Pine",   "Redwood",
    "Rosewood",   "Spruce",   "Sycamore", "Teak",   "Walnut",
};

static char* metals[] = {
    "Aluminum",      "Cast Iron",    "Chromium",
    "Copper",        "Gold",         "Iron",
    "Magnesium",     "Molybdenum",   "Nickel",
    "Rusty",         "Silver",       "Steel",
    "Tin",           "Titanium",     "Tungsten",
    "Zirconium",     "Zinc",         "Aluminum-Plated",
    "Copper-Plated", "Gold-Plated",  "Nickel-Plated",
    "Silver-Plated", "Steel-Plated", "Tin-Plated",
    "Zinc-Plated",
};

static char* rocks[] = {
    "Alexandrite", "Amethyst",     "Aquamarine", "Azurite",   "Beryl",
    "Bloodstone",  "Calcite",      "Carnelian",  "Corundum",  "Diamond",
    "Emerald",     "Fluorite",     "Garnet",     "Granite",   "Jade",
    "Jasper",      "Lapis Lazuli", "Malachite",  "Marble",    "Moonstone",
    "Onyx",        "Opal",         "Pearl",      "Quartz",    "Quartzite",
    "Rhodonite",   "Ruby",         "Sapphire",   "Tiger Eye", "Topaz",
    "Turquoise",   "Zircon",
};

static char* amulets[] = {
    "Amber", "Driftwood", "Coral",  "Agate",  "Ivory",          "Obsidian",
    "Bone",  "Brass",     "Bronze", "Pewter", "Tortoise Shell",
};

static char* syllableD[] = {
    "a",    "ab",   "ag",   "aks",  "ala",  "an",  "ankh", "app", "arg",
    "arze", "ash",  "aus",  "ban",  "bar",  "bat", "bek",  "bie", "bin",
    "bit",  "bjor", "blu",  "bot",  "bu",   "byt", "comp", "con", "cos",
    "cre",  "dalf", "dan",  "den",  "doe",  "dok", "eep",  "el",  "eng",
    "er",   "ere",  "erk",  "esh",  "evs",  "fa",  "fid",  "for", "fri",
    "fu",   "gan",  "gar",  "glen", "gop",  "gre", "ha",   "he",  "hyd",
    "i",    "ing",  "ion",  "ip",   "ish",  "it",  "ite",  "iv",  "jo",
    "kho",  "kli",  "klis", "la",   "lech", "man", "mar",  "me",  "mi",
    "mic",  "mik",  "mon",  "mung", "mur",  "nej", "nelg", "nep", "ner",
    "nes",  "nis",  "nih",  "nin",  "o",    "od",  "ood",  "org", "orn",
    "ox",   "oxy",  "pay",  "pet",  "ple",  "plu", "po",   "pot", "prok",
    "re",   "rea",  "rhov", "ri",   "ro",   "rog", "rok",  "rol", "sa",
    "san",  "sat",  "see",  "sef",  "seh",  "shu", "ski",  "sna", "sne",
    "snik", "sno",  "so",   "sol",  "sri",  "sta", "sun",  "ta",  "tab",
    "tem",  "ther", "ti",   "tox",  "trol", "tue", "turs", "u",   "ulk",
    "um",   "un",   "uni",  "ur",   "val",  "viv", "vly",  "vom", "wah",
    "wed",  "werg", "wex",  "whon", "wun",  "x",   "yerg", "yp",  "zun",
};
static char titleD[MAX_SUBVAL][10];
