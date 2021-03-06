#include "ImageBase.h"

////////////////////////////////// ImageBase ////////////////////////////////////////

ImageBase::ImageBase() : m_height(0), m_width(0), m_valid(true) {}

ImageBase::ImageBase(size_t width, size_t height) : m_height(width), m_width(height), m_valid(true) {}

ImageBase::ImageBase(const std::string& filename) : m_height(0), m_width(0), m_valid(true) {
	this->load(filename);
}

size_t ImageBase::height() const {
	return m_height;
}

size_t ImageBase::width() const {
	return m_width;
}

size_t ImageBase::size() const {
	return this->width() * this->height();
}

bool ImageBase::valid() const {
	return m_valid;
}

bool ImageBase::on_bounds(size_t raw, size_t column) const {
	return raw < this->height() && column < this->width();
}

bool ImageBase::same_resolution(const ImageBase& image) const {
	return this->height() == image.height() && this->width() == image.width();
}

void ImageBase::load(const std::string& filename) {

	std::ifstream in_file(filename);
	
	if (!in_file.is_open()) {
		std::cerr << "Error : failure at load while opening file\n";
		return;
	}

	this->read(in_file);
	in_file.close();
}

void ImageBase::save(const std::string& filename) {

	std::ofstream out_file(filename);

	if (!out_file.is_open()) {
		std::cerr << "Error : failure at save while opening file\n";
		return;
	}

	this->write(out_file);
	out_file.close();
}

std::istream& operator>>(std::istream& is, ImageBase& image) {
	image.read(is);
	return is;
}

std::ostream& operator<<(std::ostream& os, const ImageBase& image) {
	image.write(os);
	return os;
}

////////////////////////////////// ImagePGM ////////////////////////////////////////

ImagePGM::ImagePGM() : ImageBase(), m_data() {}

ImagePGM::ImagePGM(size_t width, size_t height) : ImageBase(width, height), m_data(height * width) {}

ImagePGM::ImagePGM(const std::string& filename) : ImageBase(), m_data() {
	this->load(filename);
}

void ImagePGM::resize(size_t width, size_t height) {
	m_data.resize(width * height);
}

OCTET& ImagePGM::operator[](size_t n) {
	return m_data[n];
}

const OCTET& ImagePGM::operator[](size_t n) const {
	return m_data[n];
}

OCTET& ImagePGM::operator()(size_t raw, size_t column) {
	return this->m_data[raw * this->width() + column]; 
}

const OCTET& ImagePGM::operator()(size_t raw, size_t column) const {
	return (*this)(raw, column);
}

OCTET* ImagePGM::data() {
	return m_data.data(); 
}

const OCTET* ImagePGM::data() const {
	return m_data.data();
}

ImagePGM::iterator ImagePGM::begin() {
	return this->data();
}

ImagePGM::const_iterator ImagePGM::begin() const {
	return this->begin();
}

ImagePGM::iterator ImagePGM::end() {
	return this->data() + this->size();
}

ImagePGM::const_iterator ImagePGM::end() const {
	return this->end();
}

void ignore_comment(std::istream& is) {
  OCTET c;
  while ((c = is.get()) == '#')
    while ((c = is.get()) != '\n')
   		;
  is.seekg(-sizeof(OCTET), is.cur);
}

void ImagePGM::read(std::istream& is) {
	
	std::string fmt;
	int max_grey_val;

	is >> fmt; //P5
	ignore_comment(is);
	is >> this->m_width >> this->m_height;
	is >> max_grey_val;

	this->m_data.resize(m_height * m_width);

	is.read((char*)this->data(), this->size());

	m_valid = is.good();
}

void ImagePGM::write(std::ostream& os) const {

	std::string fmt;

	os << "P5\r";
	os << this->width() << " " << this->height() << "\r";
	os << "255\r";

	os.write((char*)this->data(), this->size());

	if (!os.good())
		std::cerr << "Warning : failure at save while writing in file\n";
}

////////////////////////////////// ImagePPM ////////////////////////////////////////

ImagePPM::ImagePPM() : ImageBase() {}

ImagePPM::ImagePPM(size_t width, size_t height) : ImageBase(width, height) {}

ImagePPM::ImagePPM(const std::string& filename) : ImageBase() {
	this->load(filename);
}

ImagePPM::ImagePPM(const ImagePGM& red, const ImagePGM& green, const ImagePGM& blue) {
	
	if (red.same_resolution(green) && red.same_resolution(blue)) {
		this->m_red = red;
		this->m_green = green;
		this->m_blue = blue;
	}
}

OCTET& ImagePPM::red(size_t raw, size_t column) {
	return this->m_red(raw, column);
}

const OCTET& ImagePPM::red(size_t raw, size_t column) const {
	return this->m_red(raw, column);
}

OCTET& ImagePPM::green(size_t raw, size_t column) {
	return this->m_green(raw, column);
}

const OCTET& ImagePPM::green(size_t raw, size_t column) const {
	return this->m_green(raw, column);
}

OCTET& ImagePPM::blue(size_t raw, size_t column) {
	return this->m_blue(raw, column);
}

const OCTET& ImagePPM::blue(size_t raw, size_t column) const {
	return this->m_blue(raw, column);
}

const ImagePGM& ImagePPM::red() const {
	return this->red();
}

const ImagePGM& ImagePPM::green() const {
	return this->green();
}

const ImagePGM& ImagePPM::blue() const {
	return this->blue();
}

void ImagePPM::red(const ImagePGM& red) {
	if (this->same_resolution(red))
		this->m_red = red;
}

void ImagePPM::green(const ImagePGM& green) {
	if (this->same_resolution(green))
		this->m_green = green;
}

void ImagePPM::blue(const ImagePGM& blue) {
	if (this->same_resolution(blue))
		this->m_blue = blue;
}

void ImagePPM::read(std::istream& is) {

	std::string fmt;
	int max_grey_val;

	is >> fmt; //P6
	ignore_comment(is);
	is >> this->m_width >> this->m_height;
	is >> max_grey_val;

	std::vector<OCTET> data(this->width() * this->height() * 3);

	is.read((char*)data.data(), data.size());

	this->m_red.resize(this->width(), this->height());
	this->m_green.resize(this->width(), this->height());
	this->m_blue.resize(this->width(), this->height());

	for (size_t i = 0 ; i < this->size(); ++i) {
		this->m_red[i] = data[i*3];
		this->m_green[i] = data[i*3+1];
		this->m_blue[i] = data[i*3+2];
	}

	m_valid = is.good();
}

void ImagePPM::write(std::ostream& os) const {

	std::string fmt;

	os << "P6\r";
	os << this->width() << " " << this->height() << "\r";
	os << "255\r";

	std::vector<OCTET> data(this->width() * this->height() * 3);

	for (size_t i = 0 ; i < this->size() ; ++i) {
		data[i*3] = this->m_red[i];
		data[i*3+1] = this->m_green[i];
		data[i*3+2] = this->m_blue[i];
	}

	os.write((char*)data.data(), data.size());

	os << "\n";

	if (!os.good())
		std::cerr << "Warning : failure at save while writing in file\n";
}
