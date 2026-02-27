for (int i = 0; i < 10; i++) {
    std::cout << "Enter password: ";
    std::string input;
    std::cin >> input;

    if (input == "password123") {
        std::cout << "Access granted!" << std::endl;
        break;
    } else {
        std::cout << "Incorrect password. Try again." << std::endl;
    }
}