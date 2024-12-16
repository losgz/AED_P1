import os
import random
import math


def generate_binary_pbm(size, output_path, pattern="random"):
    """
    Generate a binary PBM image (P4 format) with a specified pattern.

    Parameters:
    size (int): Dimensions of the square image (size x size).
    output_path (str): Path to save the generated PBM image.
    pattern (str): Pattern type - "best", "worst", or "random".
    """
    try:
        with open(output_path, "wb") as f:
            # Write PBM header (P4 format)
            f.write(b"P4\n")
            f.write(f"{size} {size}\n".encode())

            # Generate binary pixel data
            row_data = []
            for row in range(size):
                if pattern == "best":
                    # All 1s for the best case (completely black)
                    line = [1] * size
                elif pattern == "worst":
                    # Chessboard pattern for the worst case
                    line = [(row + col) % 2 for col in range(size)]
                elif pattern == "random":
                    # Random pattern
                    line = [random.randint(0, 1) for _ in range(size)]
                else:
                    raise ValueError(f"Unknown pattern: {pattern}")

                # Convert the line into binary bytes
                byte_line = []
                for i in range(0, size, 8):
                    # Pack each group of 8 pixels into a byte
                    byte = 0
                    for bit in line[i : i + 8]:
                        byte = (byte << 1) | bit
                    byte_line.append(byte)
                row_data.append(bytearray(byte_line))

            # Write all rows
            for row in row_data:
                f.write(row)
        print(f"{pattern.capitalize()} case PBM image saved to {output_path}")
    except Exception as e:
        print(f"Error generating PBM file: {e}")


def generate_test_images(size, output_dir, case_type, i_num):
    """
    Generate multiple PBM images for a specific case.

    Parameters:
    size (int): Dimensions of the square image (size x size).
    output_dir (str): Directory to save the images.
    case_type (str): Type of case ("best", "worst", "random").
    """
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"{case_type}_case_image_{i_num}.pbm")
    generate_binary_pbm(size, output_path, pattern=case_type)


# Main script to generate best, worst, and average case images
if __name__ == "__main__":
    num_images = 5  # Number of random images per size
    image_size = 16
    output_directory = "binary_pbm_images"
    image_num = 1

    while image_size <= 1028:
        # Best case: All black (1s)
        generate_test_images(image_size, output_directory, "best", image_num)
        image_num += 1

        # Worst case: Chessboard pattern
        generate_test_images(image_size, output_directory, "worst", image_num)
        image_num += 1

        # Average case: Random patterns
        for _ in range(num_images):
            generate_test_images(image_size, output_directory, "random", image_num)
            image_num += 1

        image_size *= 2
