<h2>Description</h2>
<p>This project leverages the power of indexing and file structures to maintain and manage healthcare records. It includes separate files for storing patient details, doctor information, appointment availability, and both primary and secondary indexes for fast lookups. Written in C++, the HCMS ensures data consistency and ease of access through its modular design and comprehensive file handling.</p>
<p>The Health Care Management System (HCMS) is designed to efficiently manage healthcare resources such as patient records, doctor schedules, and appointment bookings. This system utilizes a structured approach with primary and secondary indexes, ensuring optimized data organization and retrieval. It is a lightweight, file-based solution suitable for small to medium-sized healthcare facilities.</p>

<h2>File Structure</h2>
<ul>
    <li><strong>.vscode/</strong> - Contains configuration files for Visual Studio Code.</li>
    <li><strong>Names&amp;IDs.pdf</strong> - A PDF file with names and IDs of relevant stakeholders (e.g., librarians or members).</li>
    <li><strong>books.avail</strong> - File storing book availability data.</li>
    <li><strong>books.index</strong> - File indexing book-related information.</li>
    <li><strong>books_secondary.index</strong> - Secondary indexing file for book data.</li>
    <li><strong>books.txt</strong> - Text file storing detailed information about all books.</li>
    <li><strong>users.avail</strong> - File storing user membership and availability data.</li>
    <li><strong>users.index</strong> - File indexing user-related information.</li>
    <li><strong>users_secondary.index</strong> - Secondary indexing file for user data.</li>
    <li><strong>users.txt</strong> - Text file containing detailed information about all users.</li>
    <li><strong>main.cpp</strong> - The main source code file for the Library Management System.</li>
    <li><strong>main.exe</strong> - Compiled executable of the main program.</li>
</ul>

<h2>Features</h2>
<ul>
    <li>Manage book profiles and availability.</li>
    <li>Maintain user membership details efficiently.</li>
    <li>Organize and search data using primary and secondary indexing for faster retrieval.</li>
    <li>Ensure data integrity with structured file systems.</li>
</ul>

<h2>How to Run</h2>
<ol>
    <li><strong>Compilation</strong>:<br>
        Make sure you have a C++ compiler installed (e.g., <code>g++</code>). Compile the <code>main.cpp</code> file using the command:
        <pre><code>g++ -o main.exe main.cpp</code></pre>
    </li>
    <li><strong>Execution</strong>:<br>
        Run the compiled executable:
        <pre><code>./main.exe</code></pre>
    </li>
    <li><strong>Configuration</strong>:<br>
        Ensure all the data files (e.g., <code>books.avail</code>, <code>users.txt</code>, etc.) are in the same directory as the executable.</li>
</ol>

<h2>Prerequisites</h2>
<ul>
    <li>A C++ compiler (e.g., GCC or Clang)</li>
    <li>A PDF reader to view <code>Names&amp;IDs.pdf</code></li>
    <li>Text editor or IDE (optional, for editing source code)</li>
</ul>

<h2>Contributing</h2>
<p>If you'd like to contribute to this project:</p>
<ol>
    <li>Fork the repository.</li>
    <li>Create a new branch for your feature or bug fix.</li>
    <li>Commit your changes and push them to your fork.</li>
    <li>Create a pull request to the main branch of this repository.</li>
</ol>

<h2>License</h2>
<p>This project is licensed under the MIT License. See the LICENSE file for more details.</p>

<h2>Contact</h2>
<p>For any questions or support, please contact the repository owner or create an issue in the repository.</p>
