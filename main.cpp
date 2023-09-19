#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <ostream>

using namespace std;

struct Rating {
    int userId;
    int movieId;
    float rating;
};

// Struct to represent a user
struct User {
    int userId;
    std::map<int, float> ratings;
};

// Struct to represent a movie
struct Movie {
    int movieId;
    std::map<int, float> ratings;
};

std::vector<Rating> readRatings(std::string fileName) {
    std::vector<Rating> ratings;
    std::ifstream file(fileName);
    std::string line;
    // Skip the first line
    std::getline(file, line);
    // Iterate through the remaining lines
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        Rating rating;
        try {
            // Extract userId, movieId, and rating from the line
            std::getline(lineStream, cell, ',');
            rating.userId = std::stoi(cell);
            std::getline(lineStream, cell, ',');
            rating.movieId = std::stoi(cell);
            std::getline(lineStream, cell, ',');
            rating.rating = std::stof(cell);
            // Add the rating to the ratings vector
            ratings.push_back(rating);
        }
        catch (const std::invalid_argument& e) {
            // Ignore invalid input
            continue;
        }
    }

    return ratings;

}

// Function to compute the cosine similarity between two users
float computeCosineSimilarity(User user1, User user2) {
    float dotProduct = 0.0;
    float norm1 = 0.0;
    float norm2 = 0.0;
    // Consider all movies rated by either user
    for (auto&& [movieId, rating] : user1.ratings) {
        norm1 += rating * rating;
        if (user2.ratings.count(movieId)) {
            dotProduct += rating * user2.ratings[movieId];
        }
    }
    for (auto&& [movieId, rating] : user2.ratings) {
        norm2 += rating * rating;
    }
    if (norm1 == 0.0 || norm2 == 0.0) {
        // Return 0 if one of the users has no ratings
        return 0.0;
    }
    return dotProduct / (sqrt(norm1) * sqrt(norm2));
}
float computeCosineSimilarity(Movie movie1, Movie movie2) {
    float dotProduct = 0.0;
    float norm1 = 0.0;
    float norm2 = 0.0;
    // Consider all users who have rated either movie
    for (auto&& [userId, rating] : movie1.ratings) {
        norm1 += rating * rating;
        if (movie2.ratings.count(userId)) {
            dotProduct += rating * movie2.ratings[userId];
        }
    }
    for (auto&& [userId, rating] : movie2.ratings) {
        norm2 += rating * rating;
    }
    if (norm1 == 0.0 || norm2 == 0.0) {
        // Return 0 if one of the movies has no ratings
        return 0.0;
    }
    return dotProduct / (sqrt(norm1) * sqrt(norm2));
}

float predictRating(User testUser, std::map<int, User> trainUsers, int movieId) {
    float prediction = 0.0;
    float similaritySum = 0.0;
    // Iterate through all users
    for (auto&& [userId, trainUser] : trainUsers) {
        // Skip the test user
        if (testUser.userId == trainUser.userId) {
            continue;
        }
        // If the user has already rated the movie, just return their rating
        if (trainUser.ratings.count(movieId)) {
            return trainUser.ratings[movieId];
        }
        float similarity = computeCosineSimilarity(testUser, trainUser);
        prediction += similarity * trainUser.ratings[movieId];
        similaritySum += similarity;
    }
    // Return the predicted rating
    return prediction / similaritySum;
}

float predictRating(Movie testMovie, std::map<int, Movie> trainMovies, int userId) {
    float prediction = 0.0;
    float similaritySum = 0.0;
    // Iterate through all movies
    for (auto&& [movieId, trainMovie] : trainMovies) {
        // Skip the test movie
        if (testMovie.movieId == trainMovie.movieId) {
            continue;
        }
        // If the user has already rated the movie, just return their rating
        if (trainMovie.ratings.count(userId)) {
            return trainMovie.ratings[userId];
        }
        float similarity = computeCosineSimilarity(testMovie, trainMovie);
        prediction += similarity * trainMovie.ratings[userId];
        similaritySum += similarity;
    }
    // Return the predicted rating
    return prediction / similaritySum;
}

int main() {
    // Read in ratings from train.csv
    std::vector<Rating> ratings = readRatings("./datafiles/train.csv");
    // Create maps of users and movies to their ratings
    std::map<int, User> users;
    std::map<int, Movie> movies;
    for (const Rating &rating: ratings) {
        if (!users.count(rating.userId)) {
            users[rating.userId] = {rating.userId};
        }
        users[rating.userId].ratings[rating.movieId] = rating.rating;
        if (!movies.count(rating.movieId)) {
            movies[rating.movieId] = {rating.movieId};
        }
        movies[rating.movieId].ratings[rating.userId] = rating.rating;
    }

    // Open test.csv and conclusion.csv files
    std::ifstream testFile("./datafiles/test.csv");
    std::ofstream conclusionFile("./datafiles/conclusion.csv");
    std::string line;
    // Skip the first line
    std::getline(testFile, line);
    // Write the header line to conclusion.csv
    conclusionFile << "userId,movieId,prediction" << std::endl;
    // Iterate through the remaining lines
    while (std::getline(testFile, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        int userId;
        int movieId;
        try {
            // Extract userId and movieId from the line
            std::getline(lineStream, cell, ',');
            userId = std::stoi(cell);
            std::getline(lineStream, cell, ',');
            movieId = std::stoi(cell);
            // Predict the rating using both user-based and item-based recommendations
            float userBasedPrediction = predictRating(users[userId], users, movieId);
            float itemBasedPrediction = predictRating(movies[movieId], movies, userId);
            float prediction = (userBasedPrediction + itemBasedPrediction) / 2.0;
            // Write the prediction to conclusion.csv
            conclusionFile << userId << "," << movieId << "," << prediction << std::endl;
        }
        catch (const std::invalid_argument& e) {
            // Ignore invalid input
            continue;
        }
    }

    return 0;
}



