#include "SolanaManager.h"
#include "Misc/MessageDialog.h"
#include "Engine/Engine.h"
#include "solana_sdk.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// Constructor: Initialize Solana Client
USolanaManager::USolanaManager()
{
    SolanaClient = nullptr; // Initialize as null
    UE_LOG(LogTemp, Log, TEXT("USolanaManager constructed."));
}

// Destructor: Free the Solana client
USolanaManager::~USolanaManager()
{
    if (SolanaClient)
    {
        free_client(SolanaClient);
        SolanaClient = nullptr;
        UE_LOG(LogTemp, Log, TEXT("SolanaClient freed in USolanaManager destructor."));
    }
}

// Static function to create an instance of SolanaManager
USolanaManager *USolanaManager::CreateSolanaManager()
{
    USolanaManager *Manager = NewObject<USolanaManager>();
    if (Manager)
    {
        UE_LOG(LogTemp, Log, TEXT("USolanaManager instance created successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create USolanaManager instance."));
    }
    return Manager;
}

// Connect to Solana RPC
void USolanaManager::ConnectToSolana(const FString &RpcUrl)
{
    FTCHARToUTF8 UTF8Converter(*RpcUrl);
    const char *CUrl = UTF8Converter.Get();

    SolanaClient = new_sol_client(CUrl);

    if (SolanaClient)
    {
        UE_LOG(LogTemp, Log, TEXT("Connected to Solana RPC at: %s"), *RpcUrl);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to Solana RPC at: %s"), *RpcUrl);
    }
}

// Create a new wallet
FSolanaKeyPair USolanaManager::CreateWallet()
{
    FSolanaKeyPair Wallet;

    SolKeyPair *KeyPair = new_keypair();
    if (KeyPair)
    {
        // Copy private key
        for (int i = 0; i < 64; i++)
        {
            Wallet.PrivateKey[i] = KeyPair->bytes[i];
        }

        // Copy public key
        for (int i = 0; i < 32; i++)
        {
            Wallet.PublicKey.Data[i] = KeyPair->pubkey.data[i];
        }

        UE_LOG(LogTemp, Log, TEXT("New Wallet Created:"));
        UE_LOG(LogTemp, Log, TEXT("Public Key: %s"), *FString::FromHexBlob(Wallet.PublicKey.Data.GetData(), 32));
        UE_LOG(LogTemp, Log, TEXT("Private Key: %s"), *FString::FromHexBlob(Wallet.PrivateKey.GetData(), 64));

        free_payer(KeyPair);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create a new wallet."));
    }

    return Wallet;
}

// Retrieve balance for a wallet
void USolanaManager::GetBalance(const FSolanaPublicKey &PublicKey, FString &Balance, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return;
    }

    SolPublicKey SolPubKey;
    memcpy(SolPubKey.data, PublicKey.Data.GetData(), 32);

    uint64_t Bal = get_balance(SolanaClient, &SolPubKey);

    if (Bal == 0)
    {
        ErrorMessage = TEXT("Failed to retrieve balance. Ensure the public key is correct.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
    }
    else
    {
        Balance = FString::Printf(TEXT("%llu"), Bal);
        UE_LOG(LogTemp, Log, TEXT("Balance for public key: %s is %s"), *FString::FromHexBlob(PublicKey.Data.GetData(), 32), *Balance);
    }
}

// Display Solana Public Key
FString USolanaManager::DisplayPublicKey(const FSolanaPublicKey &PublicKey, const FString &Description)
{
    FString KeyHex = FString::FromHexBlob(PublicKey.Data.GetData(), 32);

    // Log to Output Log with description
    UE_LOG(LogTemp, Log, TEXT("%s\nPublic Key: %s"), *Description, *KeyHex);

    // Display on-screen message with description
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s\nPublic Key: %s"), *Description, *KeyHex));
    }

    // Show popup with description
    FText Title = FText::FromString(TEXT("Public Key Details"));
    FText Message = FText::FromString(FString::Printf(TEXT("%s\n\nPublic Key: %s"), *Description, *KeyHex));
    FMessageDialog::Open(EAppMsgType::Ok, Message, &Title);

    return KeyHex;
}

// Display Solana Key Pair
FString USolanaManager::DisplayKeyPair(const FSolanaKeyPair &KeyPair, const FString &Description)
{
    FString PrivateKeyHex = FString::FromHexBlob(KeyPair.PrivateKey.GetData(), 64);
    FString PublicKeyHex = FString::FromHexBlob(KeyPair.PublicKey.Data.GetData(), 32);

    // Log to Output Log with description
    UE_LOG(LogTemp, Log, TEXT("%s\nKey Pair Details:"), *Description);
    UE_LOG(LogTemp, Log, TEXT("Public Key: %s"), *PublicKeyHex);
    UE_LOG(LogTemp, Log, TEXT("Private Key: %s"), *PrivateKeyHex);

    // Display on-screen messages with description
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s\nKey Pair Details:"), *Description));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Public Key: %s"), *PublicKeyHex));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Private Key: %s"), *PrivateKeyHex));
    }

    // Show popup with description
    FText Title = FText::FromString(TEXT("Key Pair Details"));
    FText Message = FText::FromString(FString::Printf(TEXT("%s\n\nPublic Key: %s\nPrivate Key: %s"), *Description, *PublicKeyHex, *PrivateKeyHex));
    FMessageDialog::Open(EAppMsgType::Ok, Message, &Title);

    return FString::Printf(TEXT("Public Key: %s\nPrivate Key: %s"), *PublicKeyHex, *PrivateKeyHex);
}

void USolanaManager::ShowPopup(const FString &Title, const FString &Message)
{
    FText TitleText = FText::FromString(Title);
    FText MessageText = FText::FromString(Message);

    // Display the message dialog
    FMessageDialog::Open(EAppMsgType::Ok, MessageText, &TitleText);

    UE_LOG(LogTemp, Log, TEXT("Popup displayed with title: %s, message: %s"), *Title, *Message);
}

FString USolanaManager::GetWalletAddress(const FSolanaKeyPair &KeyPair)
{
    // Convert FSolanaKeyPair to SolKeyPair
    SolKeyPair Wallet;
    memcpy(Wallet.bytes, KeyPair.PrivateKey.GetData(), 64);
    memcpy(Wallet.pubkey.data, KeyPair.PublicKey.Data.GetData(), 32);

    // Call get_wallet_address to retrieve the address
    char *CAddress = get_wallet_address(&Wallet);

    if (!CAddress)
    {
        FString ErrorMessage = TEXT("Failed to retrieve wallet address.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return ErrorMessage;
    }

    // Convert C string to FString
    FString Address = FString(UTF8_TO_TCHAR(CAddress));

    // Free the allocated memory for the address
    free(CAddress);

    // Log and display the wallet address
    UE_LOG(LogTemp, Log, TEXT("Wallet Address: %s"), *Address);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Wallet Address: %s"), *Address));
    }

    return Address;
}

FSolanaKeyPair USolanaManager::CreateAndSaveWallet(const FString &FilePath, FString &ErrorMessage)
{
    FSolanaKeyPair Wallet; // Initialize the wallet struct to return

    // Determine the file path
    FString FinalFilePath = FilePath.IsEmpty()
                                ? FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Wallet.json")) // Default path
                                : FilePath;

    // Log the resolved file path
    UE_LOG(LogTemp, Log, TEXT("Saving wallet to: %s"), *FinalFilePath);

    // Convert FilePath to a C string
    FTCHARToUTF8 UTF8Converter(*FinalFilePath);
    const char *CFilePath = UTF8Converter.Get();

    // Call the Solana SDK function to create and save the wallet
    SolKeyPair *KeyPair = create_and_save_wallet(CFilePath);
    if (KeyPair)
    {
        // Copy private key
        for (int i = 0; i < 64; i++)
        {
            Wallet.PrivateKey[i] = KeyPair->bytes[i];
        }

        // Copy public key
        for (int i = 0; i < 32; i++)
        {
            Wallet.PublicKey.Data[i] = KeyPair->pubkey.data[i];
        }

        UE_LOG(LogTemp, Log, TEXT("New Wallet Created:"));
        UE_LOG(LogTemp, Log, TEXT("Public Key: %s"), *FString::FromHexBlob(Wallet.PublicKey.Data.GetData(), 32));
        UE_LOG(LogTemp, Log, TEXT("Private Key: %s"), *FString::FromHexBlob(Wallet.PrivateKey.GetData(), 64));

        free_payer(KeyPair);
    }
    else
    {
        ErrorMessage = FString::Printf(TEXT("Failed to create and save wallet to: %s"), *FinalFilePath);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
    }

    return Wallet;
}

FSolanaKeyPair USolanaManager::LoadWalletFromFile(const FString &FilePath, FString &ErrorMessage)
{
    FSolanaKeyPair Wallet; // Initialize the wallet struct to return

    // Determine the file path
    FString FinalFilePath = FilePath.IsEmpty()
                                ? FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Wallet.json")) // Default path
                                : FilePath;

    // Check if the file exists
    if (!FPaths::FileExists(FinalFilePath))
    {
        ErrorMessage = FString::Printf(TEXT("Wallet file not found at: %s"), *FinalFilePath);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return Wallet;
    }

    // Convert FilePath to a C string
    FTCHARToUTF8 UTF8Converter(*FinalFilePath);
    const char *CFilePath = UTF8Converter.Get();

    // Call the Solana SDK function to load the wallet
    SolKeyPair *KeyPair = load_wallet_from_file(CFilePath);
    if (KeyPair)
    {
        // Copy private key
        for (int i = 0; i < 64; i++)
        {
            Wallet.PrivateKey[i] = KeyPair->bytes[i];
        }

        // Copy public key
        for (int i = 0; i < 32; i++)
        {
            Wallet.PublicKey.Data[i] = KeyPair->pubkey.data[i];
        }

        UE_LOG(LogTemp, Log, TEXT("Wallet loaded successfully from: %s"), *FinalFilePath);
        UE_LOG(LogTemp, Log, TEXT("Public Key: %s"), *FString::FromHexBlob(Wallet.PublicKey.Data.GetData(), 32));
        UE_LOG(LogTemp, Log, TEXT("Private Key: %s"), *FString::FromHexBlob(Wallet.PrivateKey.GetData(), 64));

        free_payer(KeyPair); // Free the allocated memory for the SolKeyPair
    }
    else
    {
        ErrorMessage = FString::Printf(TEXT("Failed to load wallet from file: %s"), *FinalFilePath);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
    }

    return Wallet;
}

bool USolanaManager::RequestFaucet(const FSolanaPublicKey &PublicKey, int64 Amount, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    if (Amount <= 0)
    {
        ErrorMessage = TEXT("Invalid faucet amount. Amount must be greater than 0.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaPublicKey to SolPublicKey
    SolPublicKey SolPubKey;
    memcpy(SolPubKey.data, PublicKey.Data.GetData(), 32);

    // Convert int64 to uint64
    uint64 Uint64Amount = static_cast<uint64>(Amount);

    // Call the request_airdrop function
    bool Success = request_airdrop(SolanaClient, &SolPubKey, Uint64Amount);
    if (Success)
    {
        FString AmountStr = FString::Printf(TEXT("%llu lamports"), Uint64Amount);
        UE_LOG(LogTemp, Log, TEXT("Successfully requested faucet of %s for public key: %s"), *AmountStr, *FString::FromHexBlob(PublicKey.Data.GetData(), 32));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Faucet requested: %s for public key: %s"), *AmountStr, *FString::FromHexBlob(PublicKey.Data.GetData(), 32)));
        }

        return true;
    }
    else
    {
        ErrorMessage = TEXT("Faucet request failed. Possible causes: insufficient funds on the test network or invalid public key.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }
}

FSolanaPublicKey USolanaManager::GetPublicKey(const FSolanaKeyPair &KeyPair)
{
    return KeyPair.PublicKey;
}

bool USolanaManager::CreateSPLToken(const FSolanaKeyPair &PayerKeyPair, const FSolanaKeyPair &MintKeyPair, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaKeyPair to SolKeyPair for both payer and mint key pairs
    SolKeyPair Payer;
    memcpy(Payer.bytes, PayerKeyPair.PrivateKey.GetData(), 64);
    memcpy(Payer.pubkey.data, PayerKeyPair.PublicKey.Data.GetData(), 32);

    SolKeyPair Mint;
    memcpy(Mint.bytes, MintKeyPair.PrivateKey.GetData(), 64);
    memcpy(Mint.pubkey.data, MintKeyPair.PublicKey.Data.GetData(), 32);

    // Call the Solana SDK function to create the SPL token
    bool Result = create_spl_token(SolanaClient, &Payer, &Mint);

    if (Result)
    {
        UE_LOG(LogTemp, Log, TEXT("SPL Token created successfully."));
        return true;
    }
    else
    {
        ErrorMessage = TEXT("Failed to create SPL token.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }
}

bool USolanaManager::GetMintInfo(const FSolanaPublicKey &MintPublicKey, FString &MintAuthority, int64 &Supply, int32 &Decimals, bool &IsInitialized, FString &FreezeAuthority, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaPublicKey to SolPublicKey
    SolPublicKey SolPubKey;
    memcpy(SolPubKey.data, MintPublicKey.Data.GetData(), 32);

    // Call the Solana SDK function to get mint info
    SolMint *MintInfo = get_mint_info(SolanaClient, &SolPubKey);

    if (!MintInfo)
    {
        ErrorMessage = TEXT("Failed to retrieve mint information.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Extract mint details
    if (MintInfo->mint_authority)
    {
        MintAuthority = FString::FromHexBlob(MintInfo->mint_authority->data, 32);
    }
    else
    {
        MintAuthority = TEXT("None");
    }

    Supply = static_cast<int64>(MintInfo->supply);
    Decimals = static_cast<int32>(MintInfo->decimals);
    IsInitialized = MintInfo->is_initialized;

    if (MintInfo->freeze_authority)
    {
        FreezeAuthority = FString::FromHexBlob(MintInfo->freeze_authority->data, 32);
    }
    else
    {
        FreezeAuthority = TEXT("None");
    }

    // Log the mint details
    UE_LOG(LogTemp, Log, TEXT("Mint Info Retrieved Successfully:"));
    UE_LOG(LogTemp, Log, TEXT("Mint Authority: %s"), *MintAuthority);
    UE_LOG(LogTemp, Log, TEXT("Supply: %lld"), Supply);
    UE_LOG(LogTemp, Log, TEXT("Decimals: %d"), Decimals);
    UE_LOG(LogTemp, Log, TEXT("Is Initialized: %s"), IsInitialized ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Log, TEXT("Freeze Authority: %s"), *FreezeAuthority);

    // Free the memory allocated for MintInfo
    free(MintInfo);

    return true;
}

bool USolanaManager::MintSPL(const FSolanaKeyPair &PayerKeyPair, const FSolanaKeyPair &MintAuthorityKeyPair, const FSolanaPublicKey &RecipientPublicKey, int64 Amount, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    if (Amount <= 0)
    {
        ErrorMessage = TEXT("Invalid mint amount. Amount must be greater than 0.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaKeyPair to SolKeyPair for the mint authority
    SolKeyPair Payer;
    memcpy(Payer.bytes, PayerKeyPair.PrivateKey.GetData(), 64);
    memcpy(Payer.pubkey.data, PayerKeyPair.PublicKey.Data.GetData(), 32);

    // Convert FSolanaKeyPair to SolKeyPair for the mint authority
    SolKeyPair MintAuthority;
    memcpy(MintAuthority.bytes, MintAuthorityKeyPair.PrivateKey.GetData(), 64);
    memcpy(MintAuthority.pubkey.data, MintAuthorityKeyPair.PublicKey.Data.GetData(), 32);

    // Convert FSolanaPublicKey to SolPublicKey for the recipient
    SolPublicKey Recipient;
    memcpy(Recipient.data, RecipientPublicKey.Data.GetData(), 32);

    // Call the mint_spl function
    bool Success = mint_spl(SolanaClient, &Payer, &MintAuthority, &Recipient, static_cast<uint64>(Amount));
    if (Success)
    {
        FString AmountStr = FString::Printf(TEXT("%lld tokens"), Amount);
        UE_LOG(LogTemp, Log, TEXT("Successfully minted %s to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successfully minted %s to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32)));
        }

        return true;
    }
    else
    {
        ErrorMessage = TEXT("Failed to mint SPL tokens.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }
}

bool USolanaManager::GetAssociatedTokenBalance(const FSolanaPublicKey &OwnerPublicKey, const FSolanaPublicKey &MintPublicKey, int64 &Balance, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaPublicKey to SolPublicKey for the owner and mint
    SolPublicKey SolOwnerPubKey;
    memcpy(SolOwnerPubKey.data, OwnerPublicKey.Data.GetData(), 32);

    SolPublicKey SolMintPubKey;
    memcpy(SolMintPubKey.data, MintPublicKey.Data.GetData(), 32);

    // Call the SDK function to get the associated token balance
    uint64_t Result = get_associated_token_balance(SolanaClient, &SolOwnerPubKey, &SolMintPubKey);

    if (Result == UINT64_MAX) // Assuming `UINT64_MAX` indicates an error, based on the SDK's behavior
    {
        ErrorMessage = TEXT("Failed to retrieve the associated token balance.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    Balance = static_cast<int64>(Result);

    // Log the balance details
    UE_LOG(LogTemp, Log, TEXT("Associated Token Balance: %lld"), Balance);

    return true;
}

bool USolanaManager::TransferSPL(const FSolanaKeyPair &SenderKeyPair, const FSolanaPublicKey &RecipientPublicKey, const FSolanaPublicKey &MintPublicKey, int64 Amount, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    if (Amount <= 0)
    {
        ErrorMessage = TEXT("Invalid transfer amount. Amount must be greater than 0.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaKeyPair to SolKeyPair for the sender
    SolKeyPair Sender;
    memcpy(Sender.bytes, SenderKeyPair.PrivateKey.GetData(), 64);
    memcpy(Sender.pubkey.data, SenderKeyPair.PublicKey.Data.GetData(), 32);

    // Convert FSolanaPublicKey to SolPublicKey for recipient and mint
    SolPublicKey SolRecipientPubKey;
    memcpy(SolRecipientPubKey.data, RecipientPublicKey.Data.GetData(), 32);

    SolPublicKey SolMintPubKey;
    memcpy(SolMintPubKey.data, MintPublicKey.Data.GetData(), 32);

    // Call the transfer_spl function from the Solana SDK
    bool Success = transfer_spl(SolanaClient, &Sender, &SolRecipientPubKey, &SolMintPubKey, static_cast<uint64_t>(Amount));

    if (Success)
    {
        FString AmountStr = FString::Printf(TEXT("%lld tokens"), Amount);
        UE_LOG(LogTemp, Log, TEXT("Successfully transferred %s to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Transferred %s tokens to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32)));
        }

        return true;
    }
    else
    {
        ErrorMessage = TEXT("Failed to transfer SPL tokens.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }
}

bool USolanaManager::TransferSOL(const FSolanaKeyPair &SenderKeyPair, const FSolanaPublicKey &RecipientPublicKey, int64 Amount, FString &ErrorMessage)
{
    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    if (Amount <= 0)
    {
        ErrorMessage = TEXT("Invalid transfer amount. Amount must be greater than 0.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }

    // Convert FSolanaKeyPair to SolKeyPair for the sender
    SolKeyPair Sender;
    memcpy(Sender.bytes, SenderKeyPair.PrivateKey.GetData(), 64);
    memcpy(Sender.pubkey.data, SenderKeyPair.PublicKey.Data.GetData(), 32);

    // Convert FSolanaPublicKey to SolPublicKey for the recipient
    SolPublicKey Recipient;
    memcpy(Recipient.data, RecipientPublicKey.Data.GetData(), 32);

    // Call the Solana SDK function for transferring SOL
    bool Success = transfer_sol(SolanaClient, &Sender, &Recipient, static_cast<uint64_t>(Amount));

    if (Success)
    {
        FString AmountStr = FString::Printf(TEXT("%lld lamports"), Amount);
        UE_LOG(LogTemp, Log, TEXT("Successfully transferred %s to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Transferred %s SOL to recipient: %s"), *AmountStr, *FString::FromHexBlob(RecipientPublicKey.Data.GetData(), 32)));
        }

        return true;
    }
    else
    {
        ErrorMessage = TEXT("Failed to transfer SOL. Ensure the sender has sufficient balance and the recipient's public key is correct.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return false;
    }
}

TArray<FTokenInfo> USolanaManager::GetAllTokens(const FSolanaPublicKey &WalletPublicKey, FString &ErrorMessage)
{
    TArray<FTokenInfo> Tokens;

    if (!SolanaClient)
    {
        ErrorMessage = TEXT("Solana client is not initialized. Call ConnectToSolana first.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return Tokens;
    }

    // Convert FSolanaPublicKey to SolPublicKey
    SolPublicKey SolPubKey;
    memcpy(SolPubKey.data, WalletPublicKey.Data.GetData(), 32);

    // Call the SDK function to retrieve tokens
    TokenList *TokenListPtr = get_all_tokens(SolanaClient, &SolPubKey);
    if (!TokenListPtr || TokenListPtr->len == 0)
    {
        ErrorMessage = TEXT("No tokens found or failed to retrieve tokens.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        return Tokens;
    }

    // Iterate over the token list and populate the array
    for (uintptr_t i = 0; i < TokenListPtr->len; ++i)
    {
        TokenInfo *TokenData = &TokenListPtr->data[i];
        FTokenInfo Token;

        Token.MintAddress = UTF8_TO_TCHAR(TokenData->mint);
        Token.Balance = UTF8_TO_TCHAR(TokenData->balance);
        Token.OwnerAddress = UTF8_TO_TCHAR(TokenData->owner);

        Tokens.Add(Token);
    }

    // Free the token list memory
    free_token_list(TokenListPtr);

    return Tokens;
}

void USolanaManager::ShowTokensInPopup(const TArray<FTokenInfo> &Tokens, const FString &Title)
{
    FString Message;
    if (Tokens.Num() == 0)
    {
        Message = TEXT("No tokens found.");
    }
    else
    {
        for (int32 i = 0; i < Tokens.Num(); ++i)
        {
            const FTokenInfo &Token = Tokens[i];
            Message += FString::Printf(
                TEXT("Token %d:\n  Mint Address: %s\n  Balance: %s\n  Owner: %s\n\n"),
                i + 1,
                *Token.MintAddress,
                *Token.Balance,
                *Token.OwnerAddress);
        }
    }

    // Use the existing ShowPopup function
    ShowPopup(Title, Message);
}
