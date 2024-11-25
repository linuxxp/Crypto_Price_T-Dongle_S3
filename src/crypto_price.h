#ifndef CRYPTO_PRICE_H
#define CRYPTO_PRICE_H

typedef struct {
    char symbol[10];
    float price;
    float change_24h;
} crypto_price_t;

void init_crypto_price(void);
bool fetch_crypto_price(crypto_price_t* price_data);
void update_display(const crypto_price_t* price_data);

#endif
