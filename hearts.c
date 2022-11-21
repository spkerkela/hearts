#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PLAYERS 4
#define DECK_SIZE 52
#define HAND_SIZE 13
#define CARDS_GIVEN 3
#define SLEEP_MULT 0

unsigned int seed;
typedef enum SUITE { HEARTS, SPADES, DIAMONDS, CLUBS, SUITE_COUNT } suite;

char *suite_str[ SUITE_COUNT ] = { "Hearts", "Spades", "Diamonds", "Clubs" };

char *player_name[ PLAYERS ] = { "Simo", "Leo", "Martin", "Jesper" };

typedef enum FACE {
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE,
    FACE_COUNT
} face;

char *face_str[ FACE_COUNT ] = { "two",   "three", "four", "five", "six",
                                 "seven", "eight", "nine", "ten",  "Jack",
                                 "Queen", "King",  "Ace" };

typedef struct card_type {
    face face;
    suite suite;
} card;

typedef struct player_type {
    card hand[ HAND_SIZE ];
    bool played[ HAND_SIZE ];
} player;

typedef struct round_type {
    bool hearts_opened;
    int turn;
    int player_took_trick;
    unsigned short scores[ PLAYERS ];
} round;

typedef struct game_type {
    int current_round_num;
    unsigned short scores[ PLAYERS ];
    round current_round;
} game;

void sleep_game( int seconds ) {
    /*
    struct timespec ts = { .tv_sec = seconds * SLEEP_MULT };
    nanosleep( &ts, NULL );
    */
}

void print_card( card card ) {
    printf( "%s of %s\n", face_str[ card.face ], suite_str[ card.suite ] );
}

bool game_is_over( game game ) {
    for ( size_t i = 0; i < PLAYERS; i++ ) {
        if ( game.scores[ i ] >= 100 ) {
            return true;
        }
    }

    return false;
}

void shuffle( card deck[ DECK_SIZE ] ) {
    int cards_placed                         = 0;
    bool placed[ SUITE_COUNT ][ FACE_COUNT ] = { false };
    int suits_placed[ SUITE_COUNT ]          = { 0 };
    face face;
    suite suite;

    while ( cards_placed < DECK_SIZE ) {
        face  = rand() % FACE_COUNT;
        suite = rand() % SUITE_COUNT;
        if ( placed[ suite ][ face ] ) {
            continue;
        }

        card card = { .face = face, .suite = suite };

        deck[ cards_placed++ ]  = card;
        placed[ suite ][ face ] = true;
        suits_placed[ suite ]++;
    }
}

void deal_cards( card deck[ DECK_SIZE ], player players[ PLAYERS ] ) {
    short cards_dealt           = 0;
    int player_dealt[ PLAYERS ] = { 0 };
    while ( cards_dealt < DECK_SIZE ) {
        int index = cards_dealt % PLAYERS;
        players[ index ].played[ player_dealt[ index ] ] = false;
        players[ index ].hand[ player_dealt[ index ]++ ] = deck[ cards_dealt ];
        cards_dealt++;
    }
}

int starting_player( player players[ PLAYERS ] ) {
    card card;
    for ( size_t i = 0; i < PLAYERS; i++ ) {
        for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
            card = players[ i ].hand[ card_i ];
            if ( card.face == TWO && card.suite == CLUBS ) {
                return i;
            }
        }
    }
    return -1;
}

void determine_cards_to_give( int round, player players[ PLAYERS ],
                              card given_cards[ PLAYERS ][ CARDS_GIVEN ] ) {
    int round_modulo = round % PLAYERS;
    if ( round_modulo == 0 ) {
        return;
    }

    for ( size_t player_index = 0; player_index < PLAYERS; player_index++ ) {
        for ( size_t j = 0; j < CARDS_GIVEN; j++ ) {
            given_cards[ player_index ][ j ] =
                players[ player_index ].hand[ j ];
        }
    }

    int offset = 1;
    if ( round_modulo == 2 ) {
        offset = 3;
    }
    if ( round_modulo == 3 ) {
        offset = 2;
    }

    for ( size_t player_index = 0; player_index < PLAYERS; player_index++ ) {
        int target = ( player_index + offset ) % PLAYERS;
        for ( size_t j = 0; j < CARDS_GIVEN; j++ ) {
            players[ target ].hand[ j ] = given_cards[ player_index ][ j ];
        }
    }
}

void first_turn( int starter, player players[ PLAYERS ], round *round,
                 card played_cards[ PLAYERS ] ) {
    card c;
    for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
        c = players[ starter ].hand[ card_i ];
        if ( c.face == TWO && c.suite == CLUBS ) {
            players[ starter ].played[ card_i ] = true;
        }
        played_cards[ 0 ].face  = TWO;
        played_cards[ 0 ].suite = CLUBS;
    }

    printf( "%s starts the game with: ", player_name[ starter ] );
    print_card( played_cards[ 0 ] );
    sleep_game( 1 );

    int round_cards_played = 1;

    for ( size_t i = 1; i < PLAYERS; i++ ) {
        int player_index              = ( starter + i ) % PLAYERS;
        bool clubs_in_hand            = false;
        bool valid_cards[ HAND_SIZE ] = { false };

        for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
            suite current_card_suite =
                players[ player_index ].hand[ card_i ].suite;
            if ( current_card_suite == CLUBS ) {
                clubs_in_hand         = true;
                valid_cards[ card_i ] = true;
            }
        }

        if ( !clubs_in_hand ) {
            bool only_hearts_in_hand = true;
            for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
                suite suite = players[ player_index ].hand[ card_i ].suite;
                face face   = players[ player_index ].hand[ card_i ].face;

                if ( suite == SPADES && face == QUEEN ) {
                    only_hearts_in_hand = false;
                    continue;
                } else if ( suite != HEARTS ) {
                    only_hearts_in_hand   = false;
                    valid_cards[ card_i ] = true;
                }
            }

            if ( only_hearts_in_hand ) {
                for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
                    valid_cards[ card_i ] = true;
                }
            }
        }

        face smallest = FACE_COUNT;

        int card_to_play = -1;

        card iter;
        for ( size_t card_i = 0; card_i < HAND_SIZE; card_i++ ) {
            if ( valid_cards[ card_i ] ) {
                iter = players[ player_index ].hand[ card_i ];
                if ( iter.face < smallest ) {
                    smallest     = iter.face;
                    card_to_play = card_i;
                }
            }
        }
        if ( card_to_play == -1 ) {
            printf( "Starting player was %s, current player is %d, seed: %d\n",
                    player_name[ starter ], player_index, seed );
            puts( "ERROR" );
        }

        iter = players[ player_index ].hand[ card_to_play ];
        printf( "%s played: ", player_name[ player_index ] );
        print_card( iter );
        sleep_game( 1 );
        players[ player_index ].played[ card_to_play ] = true;
        played_cards[ round_cards_played ].face        = iter.face;
        played_cards[ round_cards_played ].suite       = iter.suite;
        round_cards_played++;
    }

    round->turn++;
}

card choose_first_card( player *player, round *round ) {
    card c;
    bool only_hearts_in_hand = true;
    for ( size_t i = 0; i < HAND_SIZE; i++ ) {
        if ( player->played[ i ] ) {
            continue;
        }
        c = player->hand[ i ];
        if ( c.suite != HEARTS ) {
            only_hearts_in_hand = false;
        }
    }

    face smallest_face = FACE_COUNT;
    int index          = -1;

    if ( round->hearts_opened || only_hearts_in_hand ) {
        for ( size_t i = 0; i < HAND_SIZE; i++ ) {
            if ( player->played[ i ] ) {
                continue;
            }
            c = player->hand[ i ];
            if ( c.suite == HEARTS && c.face < smallest_face ) {
                smallest_face = c.face;
                index         = i;
            }
        }
        if ( index > -1 ) {
            player->played[ index ] = true;
            round->hearts_opened    = true;
            return player->hand[ index ];
        }
    }

    for ( size_t i = 0; i < HAND_SIZE; i++ ) {
        if ( player->played[ i ] ) {
            continue;
        }
        c = player->hand[ i ];
        if ( c.face < smallest_face &&
             ( round->hearts_opened || c.suite != HEARTS ) ) {
            smallest_face = c.face;
            index         = i;
        }
    }
    c = player->hand[ index ];
    if ( c.face == QUEEN && c.suite == SPADES ) {
        round->hearts_opened = true;
    }

    player->played[ index ] = true;
    return player->hand[ index ];
}

card choose_card_to_play( player *player, card played_cards[ PLAYERS ],
                          int player_turn, round *round ) {

    card c;
    face smallest_face    = FACE_COUNT;
    int index             = -1;
    int queen_of_spades_i = -1;
    int ace_i             = -1;
    int king_i            = -1;

    card trick_starter            = played_cards[ 0 ];
    bool player_must_follow_suite = false;

    for ( size_t i = 0; i < HAND_SIZE; i++ ) {
        if ( player->played[ i ] ) {
            continue;
        }

        c = player->hand[ i ];
        if ( c.suite == SPADES ) {
            if ( c.face == ACE ) {
                ace_i = i;
            } else if ( c.face == KING ) {
                king_i = i;
            } else if ( c.face == QUEEN ) {
                queen_of_spades_i = i;
            }
        }

        if ( c.suite == trick_starter.suite ) {
            player_must_follow_suite = true;
        }
    }

    if ( player_must_follow_suite ) {
        if ( trick_starter.suite == SPADES ) {
            if ( queen_of_spades_i > -1 ) {
                /* If we have queen of spades, see if anyone played king or ace
                 * and play queen if true
                 */
                for ( size_t i = 0; i < player_turn; i++ ) {
                    c = played_cards[ i ];
                    if ( ( c.face == KING || c.face == ACE ) &&
                         c.suite == SPADES ) {

                        printf( "An opportunity arises..\n" );
                        sleep_game( 2 );
                        player->played[ queen_of_spades_i ] = true;
                        round->hearts_opened                = true;
                        return player->hand[ queen_of_spades_i ];
                    }
                }
            }

            /* if last to play and we are playing spades, we should dump ace or
             * kings if we have it and no one has played queen
             */
            if ( player_turn == ( PLAYERS - 1 ) &&
                 trick_starter.suite == SPADES ) {
                for ( size_t i = 0; i < HAND_SIZE; i++ ) {
                    if ( player->played[ i ] ) {
                        continue;
                    }
                    c = player->hand[ i ];
                }
                if ( ace_i > -1 || king_i > -1 ) {
                    bool should_play_highest_spade = true;
                    for ( size_t i = 0; i < player_turn; i++ ) {
                        c = played_cards[ i ];
                        if ( c.face == QUEEN && c.suite == SPADES ) {
                            should_play_highest_spade = false;
                            break;
                        }
                    }
                    if ( should_play_highest_spade ) {
                        if ( ace_i > -1 ) {

                            player->played[ ace_i ] = true;
                            return player->hand[ ace_i ];
                        }
                        if ( king_i > -1 ) {

                            player->played[ king_i ] = true;
                            return player->hand[ king_i ];
                        }
                    }
                }
            }
        }
        for ( size_t i = 0; i < HAND_SIZE; i++ ) {
            if ( player->played[ i ] ) {
                continue;
            }
            c = player->hand[ i ];
            if ( c.suite == trick_starter.suite && c.face < smallest_face ) {
                smallest_face = c.face;
                index         = i;
            }
        }

        if ( index > -1 ) {
            player->played[ index ] = true;
            return player->hand[ index ];
        }
    }

    /* Play queen of spades at first safe opportunity */
    if ( queen_of_spades_i > -1 ) {
        printf( "An opportunity arises..\n" );
        sleep_game( 2 );
        player->played[ queen_of_spades_i ] = true;
        round->hearts_opened                = true;
        return player->hand[ queen_of_spades_i ];
    }

    /* Next dump ace or king of spades */
    if ( ace_i > -1 ) {
        player->played[ ace_i ] = true;
        return player->hand[ ace_i ];
    }
    if ( king_i > -1 ) {
        player->played[ king_i ] = true;
        return player->hand[ king_i ];
    }

    /* else play highest card, favor hearts */
    face biggest        = TWO;
    suite suite_to_play = SUITE_COUNT;
    for ( size_t i = 0; i < HAND_SIZE; i++ ) {
        if ( player->played[ i ] ) {
            continue;
        }
        c = player->hand[ i ];
        if ( c.face > biggest ) {
            index         = i;
            biggest       = c.face;
            suite_to_play = c.suite;
        } else if ( c.face == biggest && suite_to_play != HEARTS &&
                    c.suite == HEARTS ) {
            index         = i;
            suite_to_play = c.suite;
        }
    }
    c = player->hand[ index ];
    if ( c.suite == HEARTS ) {
        round->hearts_opened = true;
    }

    player->played[ index ] = true;
    return player->hand[ index ];
}

void regular_turn( int starter, player players[ PLAYERS ], round *round,
                   card played_cards[ PLAYERS ] ) {

    played_cards[ 0 ] = choose_first_card( &players[ starter ], round );
    printf( "%s starts trick with: ", player_name[ starter ] );
    print_card( played_cards[ 0 ] );
    sleep_game( 1 );
    for ( size_t i = 1; i < PLAYERS; i++ ) {
        int player_index  = ( starter + i ) % PLAYERS;
        played_cards[ i ] = choose_card_to_play( &players[ player_index ],
                                                 played_cards, i, round );
        printf( "%s plays ", player_name[ player_index ] );
        print_card( played_cards[ i ] );
        sleep_game( 1 );
    }

    round->turn++;
}

int determine_trick_winner( int starter, card played_cards[ PLAYERS ] ) {
    suite trick_suite = played_cards[ 0 ].suite;
    face largest_face = played_cards[ 0 ].face;
    int winner        = starter;
    card card_i;
    for ( size_t i = 1; i < PLAYERS; i++ ) {
        card_i = played_cards[ i ];
        if ( card_i.suite == trick_suite ) {
            if ( card_i.face > largest_face ) {
                largest_face = card_i.face;
                winner       = ( starter + i ) % PLAYERS;
            }
        }
    }

    return winner;
}

int calculate_trick_points( card played_cards[ PLAYERS ] ) {
    int sum = 0;
    card card_i;
    for ( size_t i = 0; i < PLAYERS; i++ ) {
        card_i = played_cards[ i ];
        if ( card_i.suite == HEARTS ) {
            sum++;
        } else if ( card_i.face == QUEEN && card_i.suite == SPADES ) {
            sum += 13;
        }
    }

    return sum;
}

void adjust_for_moonshot( round *round ) {
    int moonshot_index = -1;
    for ( size_t i = 0; i < PLAYERS; i++ ) {
        if ( round->scores[ i ] == 26 ) {
            moonshot_index     = i;
            round->scores[ i ] = 0;
        }
    }

    if ( moonshot_index >= 0 ) {
        printf( "It would seem mistakes were made..\n" );
        sleep_game( 4 );
        for ( size_t i = 0; i < PLAYERS; i++ ) {
            if ( i != moonshot_index ) {
                round->scores[ i ] = 26;
            }
        }
    }
}

void play_game( card deck[ DECK_SIZE ], player players[ PLAYERS ] ) {
    round round = { .hearts_opened = false, .turn = 0 };
    game game   = { .current_round = round, .current_round_num = 1 };

    card played_cards[ PLAYERS ]               = { 0 };
    card given_cards[ PLAYERS ][ CARDS_GIVEN ] = { 0 };
    int winner, starter;

    do {
        round.hearts_opened        = false;
        round.turn                 = 0;
        bool hearts_open_last_turn = false;

        for ( size_t i = 0; i < PLAYERS; i++ ) {
            round.scores[ i ] = 0;
        }

        shuffle( deck );
        deal_cards( deck, players );
        determine_cards_to_give( game.current_round_num, players, given_cards );

        starter = starting_player( players );
        printf( "Trick %d \n", 1 );
        first_turn( starter, players, &round, played_cards );

        winner = determine_trick_winner( starter, played_cards );

        printf( "%s won the trick.\n", player_name[ winner ] );
        sleep_game( 3 );
        game.scores[ winner ] += calculate_trick_points( played_cards );
        starter = winner;
        for ( size_t i = 0; i < 12; i++ ) {
            printf( "Trick %lu \n", i + 2 );
            regular_turn( starter, players, &round, played_cards );
            winner = determine_trick_winner( starter, played_cards );

            printf( "%s won the trick.\n", player_name[ winner ] );
            if ( round.hearts_opened && !hearts_open_last_turn ) {
                printf( "Hearts opened this trick!!\n" );
                hearts_open_last_turn = true;
            }
            sleep_game( 3 );
            round.scores[ winner ] += calculate_trick_points( played_cards );
            starter = winner;
        }
        adjust_for_moonshot( &round );

        for ( size_t i = 0; i < PLAYERS; i++ ) {
            game.scores[ i ] += round.scores[ i ];
        }

        printf( "--- ROUND %d over ---\n", game.current_round_num );
        printf( "--- round scores ---\n" );
        int sum = 0;
        for ( size_t i = 0; i < PLAYERS; i++ ) {
            printf( "%s, %d\n", player_name[ i ], round.scores[ i ] );
            sum += round.scores[ i ];
        }

        printf( "\nTOTAL: %d\n\n", sum );
        sleep_game( 3 );

        printf( "--- overall scores ---\n" );
        for ( size_t i = 0; i < PLAYERS; i++ ) {
            printf( "%s, %d\n", player_name[ i ], game.scores[ i ] );
        }
        game.current_round_num++;
        sleep_game( 2 );

    } while ( !game_is_over( game ) );
    printf( "Game over.\n" );
    return;
}

int main( void ) {
    time_t t;
    seed = (unsigned)time( &t );
    srand( seed );

    card deck[ DECK_SIZE ]    = { 0 };
    player players[ PLAYERS ] = { 0 };

    play_game( deck, players );

    return 0;
}
