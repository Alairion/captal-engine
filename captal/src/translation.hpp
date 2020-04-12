#ifndef CAPTAL_TRANSLATION_HPP_INCLUDED
#define CAPTAL_TRANSLATION_HPP_INCLUDED

#include "config.hpp"

namespace cpt
{

enum class language : std::uint32_t
{
    iso_aar =   1, //Afar
    iso_abk =   2, //Abkhazian
    iso_afr =   3, //Afrikaans
    iso_aka =   4, //Akan
    iso_amh =   5, //Amharic
    iso_ara =   6, //Arabic
    iso_arg =   7, //Aragonese
    iso_asm =   8, //Assamese
    iso_ava =   9, //Avaric
    iso_ave =  10, //Avestan
    iso_aym =  11, //Aymara
    iso_aze =  12, //Azerbaijani
    iso_bak =  13, //Bashkir
    iso_bam =  14, //Bambara
    iso_bel =  15, //Belarusian
    iso_ben =  16, //Bengali
    iso_bis =  17, //Bislama
    iso_bod =  18, //Tibetan
    iso_bos =  19, //Bosnian
    iso_bre =  20, //Breton
    iso_bul =  21, //Bulgarian
    iso_cat =  22, //Catalan, Valencian
    iso_ces =  23, //Czech
    iso_cha =  24, //Chamorro
    iso_che =  25, //Chechen
    iso_chu =  26, //Church Slavic, Church Slavonic, Old Bulgarian, Old Church Slavonic, Old Slavonic
    iso_chv =  27, //Chuvash
    iso_cor =  28, //Cornish
    iso_cos =  29, //Corsican
    iso_cre =  30, //Cree
    iso_cym =  31, //Welsh
    iso_dan =  32, //Danish
    iso_deu =  33, //German
    iso_div =  34, //Dhivehi, Divehi, Maldivian
    iso_dzo =  35, //Dzongkha
    iso_ell =  36, //Modern Greek (1453-)
    iso_eng =  37, //English
    iso_epo =  38, //Esperanto
    iso_est =  39, //Estonian
    iso_eus =  40, //Basque
    iso_ewe =  41, //Ewe
    iso_fao =  42, //Faroese
    iso_fas =  43, //Persian
    iso_fij =  44, //Fijian
    iso_fin =  45, //Finnish
    iso_fra =  46, //French
    iso_fry =  47, //Western Frisian
    iso_ful =  48, //Fulah
    iso_gla =  49, //Gaelic, Scottish Gaelic
    iso_gle =  50, //Irish
    iso_glg =  51, //Galician
    iso_glv =  52, //Manx
    iso_grn =  53, //Guarani
    iso_guj =  54, //Gujarati
    iso_hat =  55, //Haitian, Haitian Creole
    iso_hau =  56, //Hausa
    iso_hbs =  57, //Serbo-Croatian
    iso_heb =  58, //Hebrew
    iso_her =  59, //Herero
    iso_hin =  60, //Hindi
    iso_hmo =  61, //Hiri Motu
    iso_hrv =  62, //Croatian
    iso_hun =  63, //Hungarian
    iso_hye =  64, //Armenian
    iso_ibo =  65, //Igbo
    iso_ido =  66, //Ido
    iso_iii =  67, //Nuosu, Sichuan Yi
    iso_iku =  68, //Inuktitut
    iso_ile =  69, //Interlingue, Occidental
    iso_ina =  70, //Interlingua (International Auxiliary Language Association)
    iso_ind =  71, //Indonesian
    iso_ipk =  72, //Inupiaq
    iso_isl =  73, //Icelandic
    iso_ita =  74, //Italian
    iso_jav =  75, //Javanese
    iso_jpn =  76, //Japanese
    iso_kal =  77, //Greenlandic, Kalaallisut
    iso_kan =  78, //Kannada
    iso_kas =  79, //Kashmiri
    iso_kat =  80, //Georgian
    iso_kau =  81, //Kanuri
    iso_kaz =  82, //Kazakh
    iso_khm =  83, //Central Khmer, Khmer
    iso_kik =  84, //Gikuyu, Kikuyu
    iso_kin =  85, //Kinyarwanda
    iso_kir =  86, //Kirghiz, Kyrgyz
    iso_kom =  87, //Komi
    iso_kon =  88, //Kongo
    iso_kor =  89, //Korean
    iso_kua =  90, //Kuanyama, Kwanyama
    iso_kur =  91, //Kurdish
    iso_lao =  92, //Lao
    iso_lat =  93, //Latin
    iso_lav =  94, //Latvian
    iso_lim =  95, //Limburgan, Limburger, Limburgish
    iso_lin =  96, //Lingala
    iso_lit =  97, //Lithuanian
    iso_ltz =  98, //Letzeburgesch, Luxembourgish
    iso_lub =  99, //Luba-Katanga
    iso_lug = 100, //Ganda
    iso_mah = 101, //Marshallese
    iso_mal = 102, //Malayalam
    iso_mar = 103, //Marathi
    iso_mkd = 104, //Macedonian
    iso_mlg = 105, //Malagasy
    iso_mlt = 106, //Maltese
    iso_mon = 107, //Mongolian
    iso_mri = 108, //Maori
    iso_msa = 109, //Malay (macrolanguage)
    iso_mya = 110, //Burmese
    iso_nau = 111, //Nauru
    iso_nav = 112, //Navaho, Navajo
    iso_nbl = 113, //South Ndebele
    iso_nde = 114, //North Ndebele
    iso_ndo = 115, //Ndonga
    iso_nep = 116, //Nepali (macrolanguage)
    iso_nld = 117, //Dutch, Flemish
    iso_nno = 118, //Norwegian Nynorsk
    iso_nob = 119, //Norwegian Bokmål
    iso_nor = 120, //Norwegian
    iso_nya = 121, //Chewa, Chichewa, Nyanja
    iso_oci = 122, //Occitan (post 1500)
    iso_oji = 123, //Ojibwa
    iso_ori = 124, //Oriya (macrolanguage)
    iso_orm = 125, //Oromo
    iso_oss = 126, //Ossetian, Ossetic
    iso_pan = 127, //Panjabi, Punjabi
    iso_pli = 128, //Pali
    iso_pol = 129, //Polish
    iso_por = 130, //Portuguese
    iso_pus = 131, //Pashto, Pushto
    iso_que = 132, //Quechua
    iso_roh = 133, //Romansh
    iso_ron = 134, //Moldavian, Moldovan, Romanian
    iso_run = 135, //Rundi
    iso_rus = 136, //Russian
    iso_sag = 137, //Sango
    iso_san = 138, //Sanskrit
    iso_sin = 139, //Sinhala, Sinhalese
    iso_slk = 140, //Slovak
    iso_slv = 141, //Slovenian
    iso_sme = 142, //Northern Sami
    iso_smo = 143, //Samoan
    iso_sna = 144, //Shona
    iso_snd = 145, //Sindhi
    iso_som = 146, //Somali
    iso_sot = 147, //Southern Sotho
    iso_spa = 148, //Castilian, Spanish
    iso_sqi = 149, //Albanian
    iso_srd = 150, //Sardinian
    iso_srp = 151, //Serbian
    iso_ssw = 152, //Swati
    iso_sun = 153, //Sundanese
    iso_swa = 154, //Swahili (macrolanguage)
    iso_swe = 155, //Swedish
    iso_tah = 156, //Tahitian
    iso_tam = 157, //Tamil
    iso_tat = 158, //Tatar
    iso_tel = 159, //Telugu
    iso_tgk = 160, //Tajik
    iso_tgl = 161, //Tagalog
    iso_tha = 162, //Thai
    iso_tir = 163, //Tigrinya
    iso_ton = 164, //Tonga (Tonga Islands)
    iso_tsn = 165, //Tswana
    iso_tso = 166, //Tsonga
    iso_tuk = 167, //Turkmen
    iso_tur = 168, //Turkish
    iso_twi = 169, //Twi
    iso_uig = 170, //Uighur, Uyghur
    iso_ukr = 171, //Ukrainian
    iso_urd = 172, //Urdu
    iso_uzb = 173, //Uzbek
    iso_ven = 174, //Venda
    iso_vie = 175, //Vietnamese
    iso_vol = 176, //Volapük
    iso_wln = 177, //Walloon
    iso_wol = 178, //Wolof
    iso_xho = 179, //Xhosa 639-3
    iso_yid = 180, //Yiddish
    iso_yor = 181, //Yoruba
    iso_zha = 182, //Chuang, Zhuang
    iso_zho = 183, //Chinese
    iso_zul = 184, //Zulu
};

enum class country : std::uint32_t
{
    iso_afg =   4, //Afghanistan
    iso_ala = 248, //Åland Islands
    iso_alb =   8, //Albania
    iso_dza =  12, //Algeria
    iso_asm =  16, //American Samoa
    iso_and =  20, //Andorra
    iso_ago =  24, //Angola
    iso_aia = 660, //Anguilla
    iso_ata =  10, //Antarctica
    iso_atg =  28, //Antigua and Barbuda
    iso_arg =  32, //Argentina
    iso_arm =  51, //Armenia
    iso_abw = 533, //Aruba
    iso_aus =  36, //Australia
    iso_aut =  40, //Austria
    iso_aze =  31, //Azerbaijan
    iso_bhs =  44, //Bahamas
    iso_bhr =  48, //Bahrain
    iso_bgd =  50, //Bangladesh
    iso_brb =  52, //Barbados
    iso_blr = 112, //Belarus
    iso_bel =  56, //Belgium
    iso_blz =  84, //Belize
    iso_ben = 204, //Benin
    iso_bmu =  60, //Bermuda
    iso_btn =  64, //Bhutan
    iso_bol =  68, //Bolivia (Plurinational State of)
    iso_bes = 535, //Bonaire, Sint Eustatius and Saba
    iso_bih =  70, //Bosnia and Herzegovina
    iso_bwa =  72, //Botswana
    iso_bvt =  74, //Bouvet Island
    iso_bra =  76, //Brazil
    iso_iot =  86, //British Indian Ocean Territory
    iso_brn =  96, //Brunei Darussalam
    iso_bgr = 100, //Bulgaria
    iso_bfa = 854, //Burkina Faso
    iso_bdi = 108, //Burundi
    iso_cpv = 132, //Cabo Verde
    iso_khm = 116, //Cambodia
    iso_cmr = 120, //Cameroon
    iso_can = 124, //Canada
    iso_cym = 136, //Cayman Islands
    iso_caf = 140, //Central African Republic
    iso_tcd = 148, //Chad
    iso_chl = 152, //Chile
    iso_chn = 156, //China
    iso_cxr = 162, //Christmas Island
    iso_cck = 166, //Cocos (Keeling) Islands
    iso_col = 170, //Colombia
    iso_com = 174, //Comoros
    iso_cog = 178, //Congo
    iso_cod = 180, //Congo, Democratic Republic of the
    iso_cok = 184, //Cook Islands
    iso_cri = 188, //Costa Rica
    iso_civ = 384, //Côte d'Ivoire
    iso_hrv = 191, //Croatia
    iso_cub = 192, //Cuba
    iso_cuw = 531, //Curaçao
    iso_cyp = 196, //Cyprus
    iso_cze = 203, //Czechia
    iso_dnk = 208, //Denmark
    iso_dji = 262, //Djibouti
    iso_dma = 212, //Dominica
    iso_dom = 214, //Dominican Republic
    iso_ecu = 218, //Ecuador
    iso_egy = 818, //Egypt
    iso_slv = 222, //El Salvador
    iso_gnq = 226, //Equatorial Guinea
    iso_eri = 232, //Eritrea
    iso_est = 233, //Estonia
    iso_swz = 748, //Eswatini
    iso_eth = 231, //Ethiopia
    iso_flk = 238, //Falkland Islands (Malvinas)
    iso_fro = 234, //Faroe Islands
    iso_fji = 242, //Fiji
    iso_fin = 246, //Finland
    iso_fra = 250, //France
    iso_guf = 254, //French Guiana
    iso_pyf = 258, //French Polynesia
    iso_atf = 260, //French Southern Territories
    iso_gab = 266, //Gabon
    iso_gmb = 270, //Gambia
    iso_geo = 268, //Georgia
    iso_deu = 276, //Germany
    iso_gha = 288, //Ghana
    iso_gib = 292, //Gibraltar
    iso_grc = 300, //Greece
    iso_grl = 304, //Greenland
    iso_grd = 308, //Grenada
    iso_glp = 312, //Guadeloupe
    iso_gum = 316, //Guam
    iso_gtm = 320, //Guatemala
    iso_ggy = 831, //Guernsey
    iso_gin = 324, //Guinea
    iso_gnb = 624, //Guinea-Bissau
    iso_guy = 328, //Guyana
    iso_hti = 332, //Haiti
    iso_hmd = 334, //Heard Island and McDonald Islands
    iso_vat = 336, //Holy See
    iso_hnd = 340, //Honduras
    iso_hkg = 344, //Hong Kong
    iso_hun = 348, //Hungary
    iso_isl = 352, //Iceland
    iso_ind = 356, //India
    iso_idn = 360, //Indonesia
    iso_irn = 364, //Iran (Islamic Republic of)
    iso_irq = 368, //Iraq
    iso_irl = 372, //Ireland
    iso_imn = 833, //Isle of Man
    iso_isr = 376, //Israel
    iso_ita = 380, //Italy
    iso_jam = 388, //Jamaica
    iso_jpn = 392, //Japan
    iso_jey = 832, //Jersey
    iso_jor = 400, //Jordan
    iso_kaz = 398, //Kazakhstan
    iso_ken = 404, //Kenya
    iso_kir = 296, //Kiribati
    iso_prk = 408, //Korea (Democratic People's Republic of)
    iso_kor = 410, //Korea, Republic of
    iso_kwt = 414, //Kuwait
    iso_kgz = 417, //Kyrgyzstan
    iso_lao = 418, //Lao People's Democratic Republic
    iso_lva = 428, //Latvia
    iso_lbn = 422, //Lebanon
    iso_lso = 426, //Lesotho
    iso_lbr = 430, //Liberia
    iso_lby = 434, //Libya
    iso_lie = 438, //Liechtenstein
    iso_ltu = 440, //Lithuania
    iso_lux = 442, //Luxembourg
    iso_mac = 446, //Macao
    iso_mdg = 450, //Madagascar
    iso_mwi = 454, //Malawi
    iso_mys = 458, //Malaysia
    iso_mdv = 462, //Maldives
    iso_mli = 466, //Mali
    iso_mlt = 470, //Malta
    iso_mhl = 584, //Marshall Islands
    iso_mtq = 474, //Martinique
    iso_mrt = 478, //Mauritania
    iso_mus = 480, //Mauritius
    iso_myt = 175, //Mayotte
    iso_mex = 484, //Mexico
    iso_fsm = 583, //Micronesia (Federated States of)
    iso_mda = 498, //Moldova, Republic of
    iso_mco = 492, //Monaco
    iso_mng = 496, //Mongolia
    iso_mne = 499, //Montenegro
    iso_msr = 500, //Montserrat
    iso_mar = 504, //Morocco
    iso_moz = 508, //Mozambique
    iso_mmr = 104, //Myanmar
    iso_nam = 516, //Namibia
    iso_nru = 520, //Nauru
    iso_npl = 524, //Nepal
    iso_nld = 528, //Netherlands
    iso_ncl = 540, //New Caledonia
    iso_nzl = 554, //New Zealand
    iso_nic = 558, //Nicaragua
    iso_ner = 562, //Niger
    iso_nga = 566, //Nigeria
    iso_niu = 570, //Niue
    iso_nfk = 574, //Norfolk Island
    iso_mkd = 807, //North Macedonia
    iso_mnp = 580, //Northern Mariana Islands
    iso_nor = 578, //Norway
    iso_omn = 512, //Oman
    iso_pak = 586, //Pakistan
    iso_plw = 585, //Palau
    iso_pse = 275, //Palestine, State of
    iso_pan = 591, //Panama
    iso_png = 598, //Papua New Guinea
    iso_pry = 600, //Paraguay
    iso_per = 604, //Peru
    iso_phl = 608, //Philippines
    iso_pcn = 612, //Pitcairn
    iso_pol = 616, //Poland
    iso_prt = 620, //Portugal
    iso_pri = 630, //Puerto Rico
    iso_qat = 634, //Qatar
    iso_reu = 638, //Réunion
    iso_rou = 642, //Romania
    iso_rus = 643, //Russian Federation
    iso_rwa = 646, //Rwanda
    iso_blm = 652, //Saint Barthélemy
    iso_shn = 654, //Saint Helena, Ascension and Tristan da Cunha
    iso_kna = 659, //Saint Kitts and Nevis
    iso_lca = 662, //Saint Lucia
    iso_maf = 663, //Saint Martin (French part)
    iso_spm = 666, //Saint Pierre and Miquelon
    iso_vct = 670, //Saint Vincent and the Grenadines
    iso_wsm = 882, //Samoa
    iso_smr = 674, //San Marino
    iso_stp = 678, //Sao Tome and Principe
    iso_sau = 682, //Saudi Arabia
    iso_sen = 686, //Senegal
    iso_srb = 688, //Serbia
    iso_syc = 690, //Seychelles
    iso_sle = 694, //Sierra Leone
    iso_sgp = 702, //Singapore
    iso_sxm = 534, //Sint Maarten (Dutch part)
    iso_svk = 703, //Slovakia
    iso_svn = 705, //Slovenia
    iso_slb =  90, //Solomon Islands
    iso_som = 706, //Somalia
    iso_zaf = 710, //South Africa
    iso_sgs = 239, //South Georgia and the South Sandwich Islands
    iso_ssd = 728, //South Sudan
    iso_esp = 724, //Spain
    iso_lka = 144, //Sri Lanka
    iso_sdn = 729, //Sudan
    iso_sur = 740, //Suriname
    iso_sjm = 744, //Svalbard and Jan Mayen
    iso_swe = 752, //Sweden
    iso_che = 756, //Switzerland
    iso_syr = 760, //Syrian Arab Republic
    iso_twn = 158, //Taiwan, Province of China
    iso_tjk = 762, //Tajikistan
    iso_tza = 834, //Tanzania, United Republic of
    iso_tha = 764, //Thailand
    iso_tls = 626, //Timor-Leste
    iso_tgo = 768, //Togo
    iso_tkl = 772, //Tokelau
    iso_ton = 776, //Tonga
    iso_tto = 780, //Trinidad and Tobago
    iso_tun = 788, //Tunisia
    iso_tur = 792, //Turkey
    iso_tkm = 795, //Turkmenistan
    iso_tca = 796, //Turks and Caicos Islands
    iso_tuv = 798, //Tuvalu
    iso_uga = 800, //Uganda
    iso_ukr = 804, //Ukraine
    iso_are = 784, //United Arab Emirates
    iso_gbr = 826, //United Kingdom of Great Britain and Northern Ireland
    iso_usa = 840, //United States of America
    iso_umi = 581, //United States Minor Outlying Islands
    iso_ury = 858, //Uruguay
    iso_uzb = 860, //Uzbekistan
    iso_vut = 548, //Vanuatu
    iso_ven = 862, //Venezuela (Bolivarian Republic of)
    iso_vnm = 704, //Viet Nam
    iso_vgb =  92, //Virgin Islands (British)
    iso_vir = 850, //Virgin Islands (U.S.)
    iso_wlf = 876, //Wallis and Futuna
    iso_esh = 732, //Western Sahara
    iso_yem = 887, //Yemen
    iso_zmb = 894, //Zambia
    iso_zwe = 716, //Zimbabwe
};

enum class translation_encoding : std::uint32_t
{
    utf8 = 1,
    utf16 = 2,
    utf32 = 3,
};

/*

Captal translation files:
The format supports 3 encoding: UTF-8, UTF-16, UTF-32
All words are little-endian
The file is based on a source language, the source language is the language used in your workspace (C++ code, Game Editor, whatever),
and a target language, the one referred by the file. According to convention, the translation files should be named:
"{iso_language_code}_{iso_country_code}[.cpt].trans" where {iso_language_code} is the 3-letters language code as defined by the ISO-639-3 standard,
and where {iso_country_code} is the 3-letters country code as defined by the ISO-3166-3 standard. Part in square brackets ([]) is optional.
The source and the target languages can be represented by different encoding, and this can be used to optimize the file size:
UTF-8 is usually the lightest encoding for most languages that use latin alphabet. But it will generally be heavier than UTF-16 for Japanese, example:

For the word こんにちは (Konnichiwa) (~= Hello):
In UTF-8, it will be encoded into: [0xe3|0x81|0x93] [0xe3|0x82|0x93] [0xe3|0x81|0xab] [0xe3|0x81|0xa1] [0xe3|0x81|0xaf]
for a total of 15 octets
In UTF-16, it will be encoded into: [0x53|0x30] [0x93|0x30] [0x6b|0x30] [0x61|0x30] [0x6f|0x30]
for a total of 10 octets

For the word Voilà (a very common French preposition)
In UTF-8, it will be encoded into: [0x56][0x6f][0x69][0x6c][0xc3|0xa0]
for a total of 6 octets
In UTF-16, it will be encoded into: [0x56|0x00][0x6f|0x00][0x69|0x00][0x6c|0x00][0xe0|0x00]
for a total of 10 octets (you can see that 5 bytes are null)

The country code is used as a disambiguation marker for more accurate translation.
Some languages are spread all around the world (English, French, Spanish, Portuguese, ...) and each region has its own expressions and words.
Metropolitan French can be slightly different from Canadian French, same for Portugal and Brazil Protuguese, or USA and UK English, ...

You may think that country + language is enought, but no. Another disambiguation marker,
a "context" is a string that give information about the translation.
Languages are complex, so translations are, and sometimes the sense of the exact same phrase or sentence
can be translated differently in another language depending on the context where it is written/pronounced.
Example: "Voilà !"
You may have noticed that I didn't give a translation for Voilà in the encoding example. It's because it can be translated in more than 10 words in English.
That's why context is important.
The sense of the simple sentence "Voilà !" can be translated differently, based on the context:
"Voilà !" may mean:
-"Here it is!" if you present something
-"That's it!" if you want to indicate that something is done or to confirm an information
-"There you go!" if you want to indicate an exclamation
-"That's why!" if you want to confirm the reason of something
-And many more...
Voilà ! Now you know why :)
(Note that "Voilà" is usually preceded by another word (such as "et", "ben", ...) to precise it's meaning)

Constants:
See enumerations in translation.hpp: "cpt::language"; "cpt::country", "cpt::translation_encoding"
These enumerations are all unsigned 32-bits interger values written as is in the files.

Format:
Translation data are stored in sections, sections are defined by the context data and the first character of the string.
The context data and the first character are hashed with FNV-1a hash algorithm (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
then combined into a single 16-bytes array, stating with the context data hash followed by the first character hash, which is rehashed using FNV-1a.
The 64-bit integer is then used as a unique identifier for the section.

Header:
    File format detection:
        [8 bytes: "CPTTRANS"] magic word to detect file format
        [tph::version: file_version] the file version (tph::version is an uint64 composed of 3 fields: [uint16: major][uint16: minor][uint32: patch])
    General informations:
        [cpt::language: source_language]the source language
        [cpt::country: source_country] the source language country
        [cpt::translation_encoding: source_encoding] the source language encoding
        [cpt::language: target_language] the target language
        [cpt::country: target_country] the target language country
        [cpt::translation_encoding: target_encoding] the target language encoding
        [std::uint64_t: translation_count] the number of translated sentences/strings
    Parse informations:
        [std::uint64_t: section_count] the total number of sections
        [section_count occurencies] array of section
        {
            [std::uint64_t: section_id] the id is the hash value associated with this
            [std::uint64_t: section_begin] the begin of the section data in the file (in bytes)
        }
Data:
    Sections:
        [std::uint64_t section_size] number of translations in this section


*/

class translator
{
public:


private:
};

}

#endif
